#ifndef MGB_SHARED_OBJECT_STORE_HEADER_SOS_H
#define MGB_SHARED_OBJECT_STORE_HEADER_SOS_H

#include <array>
#include <type_traits>
#include <atomic>
#include <cassert>
#include <algorithm>
#include <stdexcept>

namespace mgb { namespace sos {
	constexpr const char* my_name() { return "Shared Object Store Library"; }
	using idx_t = std::intptr_t;

	namespace detail {

		template<class T>
		class Slot {
			std::aligned_storage_t<sizeof(T), alignof(T)> data;
			std::atomic_int ref_cnt{ 0 };

		public:
			template<class ... ARGS>
			bool try_create(ARGS&& ... args) {
				int i = 0;
				if (ref_cnt.compare_exchange_strong(i, 1)) {
					new(&data) T(std::forward<ARGS>(args)...);
					return true;
				}
				return false;
			}
			void add_ref() {
				ref_cnt.fetch_add(1,std::memory_order_relaxed);
			}
			void remove_ref() {
				assert(ref_cnt > 0);
				if (ref_cnt.fetch_sub(1) == 2) {
					object()->~T();
					ref_cnt = 0;
				}
			}
			T* object() { return reinterpret_cast<T*>(&data); }

			bool is_free() const { return ref_cnt.load(std::memory_order_relaxed) == 0; }
			bool is_uniquely_owned() const { return ref_cnt == 2; }
		};

		template<class T, idx_t Size>
		class Store {
			std::array<T, Size> data;
			std::atomic<typename std::array<T, Size>::iterator> last_next = { data.begin() };
			auto next_free_slot() {
				const auto start = last_next.load();
				auto pos = std::find_if(start, data.end(), [](auto&& s) {return s.is_free(); });
				if (pos != data.end())
				{
					return pos;
				}

				pos = std::find_if(data.begin(), start, [](auto&& s) {return s.is_free(); });
				if (pos != start) {
					return pos;
				}
				return data.end();
			}
		public:
			template<class ... ARGS>
			T& emplace(ARGS&& ... args)
			{
				auto pos = data.begin();
				while (!pos->try_create(std::forward<ARGS>(args)...))
				{
					pos = next_free_slot();
				}
				last_next = pos + 1;
				return *pos;
			}
		};
	}

	template<class T>
	class ConstHandle;


	template<class T>
	class Handle {
		template<class, idx_t>
		friend class SharedObjectStore;

		template<class>
		friend class ConstHandle;

		detail::Slot<T>* ptr;

		Handle(detail::Slot<T>* p)
			: ptr(p)
		{
			assert(ptr);
			ptr->add_ref();
		}
		void dec_ref() {
			if ( ptr ) {
				ptr->remove_ref();
			}
		}
	public:
		Handle(Handle&& other)
			: ptr(std::exchange(other.ptr, nullptr))
		{
		}

		Handle& operator=(Handle&& other)
		{
			dec_ref();
			ptr = std::exchange(other.ptr, nullptr);
			return *this;
		}
		T* operator->() const
		{
			assert(ptr);
			return ptr->object();
		}
		T& operator*() const
		{
			assert(ptr);
			return *ptr->object();
		}
		operator T& ()
		{
			assert(ptr);
			return ptr->object();
		}
		~Handle()
		{
			dec_ref();
		}
		bool unique() {
			assert(ptr);
			return ptr->is_uniquely_owned();
		}
		ConstHandle<T> lock() && ;
	};

	template<class T>
	class ConstHandle {

		detail::Slot<T>* ptr;

		void dec_ref() {
			if (ptr) {
				ptr->remove_ref();
			}
		}
		void inc_ref() const {
			if (ptr) {
				ptr->add_ref();
			}
		}

	public:
		ConstHandle(const ConstHandle& other)
			: ptr(other.ptr)
		{
			inc_ref();
		}
		ConstHandle(ConstHandle&& other)
			: ptr(std::exchange(other.ptr, nullptr))
		{
		}
		ConstHandle(Handle<T>&& other)
			: ptr(std::exchange(other.ptr, nullptr))
		{
		}
		ConstHandle& operator=(const ConstHandle& other)
		{
			other.inc_ref();
			dec_ref();
			ptr = other.ptr;
			return *this;
		}
		ConstHandle& operator=(ConstHandle&& other)
		{
			dec_ref();
			ptr = std::exchange(other.ptr, nullptr);
			return *this;
		}
		ConstHandle& operator=(Handle<T>&& other)
		{
			dec_ref();
			ptr = std::exchange(other.ptr, nullptr);
			return *this;
		}
		const T* operator->() const
		{
			assert(ptr);
			return ptr->object();
		}
		const T& operator*() const
		{
			assert(ptr);
			return *ptr->object();
		}
		operator T const & ()
		{
			assert(ptr);
			return ptr->object();
		}
		~ConstHandle()
		{
			dec_ref();
		}
		bool unique() {
			assert(ptr);
			return ptr->is_uniquely_owned();
		}
		Handle<T> turn_into_modifiable_handle() && {
			if (!unique()) {
				throw std::runtime_error("Could not turn const handle into modifiable handle, as const handle wasn't unique owner of resource");
			}
			return Handle<T>(std::exchange(ptr, nullptr));
		}
	};

	template<class T>
	ConstHandle<T> Handle<T>::lock() && {
		return ConstHandle<T>(std::move(*this));
	}

	template<class T, idx_t Size>
	class SharedObjectStore {
	public:
		template<class ... ARGS>
		Handle<T> create(ARGS&& ... args) {
			auto& e = store.emplace(args...);
			return Handle<T>(&e);
		}

	private:
		detail::Store < detail::Slot<T>, Size > store;
	};
}}

#endif // !MGB_SHARED_OBJECT_STORE_H_GUARD_SOS_H
