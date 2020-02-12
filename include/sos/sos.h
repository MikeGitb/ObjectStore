#ifndef MGB_SHARED_OBJECT_STORE_HEADER_SOS_H
#define MGB_SHARED_OBJECT_STORE_HEADER_SOS_H

#include <array>
#include <type_traits>
#include <atomic>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <thread>
#include <new>

namespace mgb { namespace sos {
	constexpr const char* my_name() noexcept { return "Shared Object Store Library"; }
	using idx_t = std::intptr_t;

	template<class T>
	struct bad_alloc : std::bad_alloc {
		const char* what() const noexcept override { return "No free slot in shared object store found"; }
	};

	namespace detail {

		template<class T>
		class Slot {
			std::aligned_storage_t<sizeof(T), alignof(T)> data{};
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
			void add_ref() noexcept {
				ref_cnt.fetch_add(1,std::memory_order_relaxed);
			}
			void remove_ref() noexcept {
				assert(ref_cnt > 1);
				if (ref_cnt.fetch_sub(1) == 2) {
					object()->~T();
					ref_cnt = 0;
				}
			}
			T* object() noexcept { return std::launder(reinterpret_cast<T*>(&data)); }

			bool is_free() const noexcept { return ref_cnt.load(std::memory_order_relaxed) == 0; }
			bool is_uniquely_owned() const noexcept { return ref_cnt == 2; }
		};

		template<class T, idx_t Size>
		class Store {
		public:
			std::array<T, Size> data;
			std::atomic<std::size_t> last_next = { 0 };
			auto next_free_slot() noexcept {
				const auto start = data.begin()+last_next.load();
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
			template<class ... ARGS>
			T& emplace(ARGS&& ... args)
			{
				int  fail_cnt = 0;
				auto pos = next_free_slot();
				while( pos == data.end() || !pos->try_create( std::forward<ARGS>( args )... ) )
				{
					pos = next_free_slot();
					if (pos == data.end()) {
						std::this_thread::yield();
					}
					fail_cnt++;
					if (fail_cnt > 10) {
						throw sos::bad_alloc<Store>();
					}
				}
				last_next = pos - data.begin() + 1;
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

		detail::Slot<T>* ptr = nullptr;

		Handle(detail::Slot<T>& p) noexcept
			: ptr(&p)
		{
			assert(ptr);
			ptr->add_ref();
		}
		void dec_ref() noexcept
		{
			if ( ptr ) {
				ptr->remove_ref();
			}
		}
	public:
		Handle(Handle&& other) noexcept
			: ptr(std::exchange(other.ptr, nullptr))
		{
		}

		Handle& operator=(Handle&& other) noexcept
		{
			dec_ref();
			ptr = std::exchange(other.ptr, nullptr);
			return *this;
		}
		T* operator->() noexcept
		{
			assert(ptr);
			return ptr->object();
		}
		const T* operator->() const noexcept
		{
			assert(ptr);
			return ptr->object();
		}
		T& operator*() noexcept
		{
			assert(ptr);
			return *ptr->object();
		}
		const T& operator*() const noexcept
		{
			assert(ptr);
			return *ptr->object();
		}
		operator T& () noexcept
		{
			assert(ptr);
			return *ptr->object();
		}
		operator const T& () const noexcept
		{
			assert(ptr);
			return *ptr->object();
		}
		~Handle()
		{
			dec_ref();
		}

		bool empty() const noexcept { return ptr == nullptr; }

		bool unique() const noexcept  {
			assert(ptr);
			return ptr->is_uniquely_owned();
		}
		ConstHandle<T> lock() && noexcept ;
	};

	template<class T>
	class ConstHandle {

		detail::Slot<T>* ptr = nullptr;

		void dec_ref() const noexcept {
			if (ptr) {
				ptr->remove_ref();
			}
		}
		void inc_ref() const noexcept {
			if (ptr) {
				ptr->add_ref();
			}
		}

	public:
		constexpr ConstHandle() noexcept = default;
		ConstHandle(const ConstHandle& other) noexcept
			: ptr(other.ptr)
		{
			inc_ref();
		}
		ConstHandle(ConstHandle&& other) noexcept
			: ptr(std::exchange(other.ptr, nullptr))
		{
		}
		ConstHandle(Handle<T>&& other) noexcept
			: ptr(std::exchange(other.ptr, nullptr))
		{
		}
		ConstHandle& operator=(const ConstHandle& other) noexcept
		{
			other.inc_ref();
			dec_ref();
			ptr = other.ptr;
			return *this;
		}
		ConstHandle& operator=(ConstHandle&& other) noexcept
		{
			dec_ref();
			ptr = std::exchange(other.ptr, nullptr);
			return *this;
		}
		ConstHandle& operator=(Handle<T>&& other) noexcept
		{
			dec_ref();
			ptr = std::exchange(other.ptr, nullptr);
			return *this;
		}
		const T* operator->() const noexcept
		{
			assert(ptr);
			return ptr->object();
		}
		const T& operator*() const noexcept
		{
			assert(ptr);
			return *ptr->object();
		}
		operator T const & () const noexcept
		{
			assert(ptr);
			return *ptr->object();
		}
		~ConstHandle()
		{
			dec_ref();
		}

		bool empty() const noexcept
		{
			return ptr != nullptr;
		}
		bool unique() const noexcept
		{
			assert(ptr);
			return ptr->is_uniquely_owned();
		}
		Handle<T> turn_into_modifiable_handle() &&
		{
			if (!unique()) {
				throw std::runtime_error("Could not turn const handle into modifiable handle, as const handle wasn't unique owner of resource");
			}
			return Handle<T>(std::exchange(ptr, nullptr));
		}
	};

	template<class T>
	ConstHandle<T> Handle<T>::lock() && noexcept
	{
		assert(ptr);
		return ConstHandle<T>(std::move(*this));
	}

	template<class T, idx_t Size>
	class SharedObjectStore {
	public:
		template<class ... ARGS>
		Handle<T> create(ARGS&& ... args) {
			return { store.emplace(args...) };
		}
		idx_t live_objects_approx() noexcept {
			return std::count_if(store.data.begin(), store.data.end(), [](const auto& s) { return !s.is_free(); });
		}
		idx_t remaining_capacity_approx() const noexcept
		{
			return std::count_if(store.data.begin(), store.data.end(), [](const auto& s) { return s.is_free(); });
		}
		constexpr idx_t capacity() noexcept { return Size; }

	private:
		detail::Store < detail::Slot<T>, Size > store;
	};
}}

#endif // !MGB_SHARED_OBJECT_STORE_H_GUARD_SOS_H
