#ifndef MGB_SHARED_OBJECT_STORE_HEADER_SOS_H
#define MGB_SHARED_OBJECT_STORE_HEADER_SOS_H

#include <array>
#include <type_traits>
#include <atomic>
#include <cassert>

namespace mgb::sos {
	constexpr const char* my_name() { return "Shared Object Store Library";  }
	using idx_t = std::intptr_t;

	namespace detail {

		template<class T>
		class Slot {
			std::aligned_storage_t<sizeof(T), alignof(T)> data;
			std::atomic_int ref_cnt{ 0 };
		public:

			template<class ... ARGS>
			void create(ARGS&& ... args) {
				new(&data) T(std::forward<ARGS>(args)...);
			}
			void destroy() {
				object().~T();
			}
			T& object() { return *reinterpret_cast<T*>(&data); }
			T* operator->() { return &object(); }
			std::atomic_int& ref_count() { return ref_cnt; }
			bool is_free() { return ref_cnt == 0; }
		};

		template<class T, idx_t Size>
		class Store {
			std::array<T, Size> data;
			auto next_free_slot() {
				return std::find_if(data.begin(), data.end(), [](auto&& s) {return s.is_free(); });
			}
		public:
			template<class ... ARGS>
			T& emplace(ARGS&& ... args)
			{
				auto pos = next_free_slot();
				assert(pos != data.end());
				pos->create(std::forward<ARGS>(args)...);
				return *pos;
			}
		};
	}

	template<class T, idx_t Size>
	class SharedObjectStore {


	public:
		class Handle {
			friend SharedObjectStore;
			detail::Slot<T>* ptr;

			Handle(detail::Slot<T>* p) : ptr(p) {
				ptr->ref_count()++;
			}
			void dec_ref() const {
				auto& ref_cnt = ptr->ref_count();
				if (ref_cnt-- == 1) {
					ptr->destroy();
				}
			}
			void inc_ref() const {
				ptr->ref_count()++;
			}

		public:
			Handle(const Handle& other)
				: ptr(other.ptr)
			{
				inc_ref();
			}
			Handle(Handle&& other)
				: ptr(std::exchange(other.ptr,nullptr))
			{
			}
			Handle& operator=(const Handle& other)
			{
				other.inc_ref();
				dec_ref();
				ptr = other.ptr;
				return *this;
			}
			Handle& operator=(Handle&& other)
			{
				dec_ref();
				ptr = std::exchange(pther.ptr,nullptr);
			}


			T * operator->() const { return &ptr->object(); }
			~Handle()
			{
				auto& ref_cnt = ptr->ref_count();
				if (ref_cnt-- == 1) {
					ptr->destroy();
				}
			}
		};

		template<class ... ARGS>
		Handle create(ARGS&& ... args) {
			auto& e = store.emplace(args...);
			return Handle(&e);
		}

		private:
			detail::Store < detail::Slot<T>, Size > store;
	};
}


#endif // !MGB_SHARED_OBJECT_STORE_H_GUARD_SOS_H
