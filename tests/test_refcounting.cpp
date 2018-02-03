#include <sos/sos.h>

#include <catch2/catch.hpp>

#include <string>
#include <atomic>
#include <vector>
#include <iostream>

using namespace mgb;

namespace {
	//The tag is included to allow each test to have its own static count, allowing to run the tests in parallel and unaffected from each other
	template<class Tag>
	struct MyType {
		static std::atomic_int i;
		std::string message;
		MyType() { i++; }
		MyType(std::string s)
			: message(std::move(s))
		{
			i++;
		}

		~MyType()
		{
			i--;
		}
		static int getCount() { return i; }
	};

	template<class Tag>
	std::atomic_int MyType<Tag>::i{ 0 };
}

struct TagTest1 {};
TEST_CASE("create_multiple_objects", "[refcounting]") {

	using E = MyType<TagTest1>;
	sos::SharedObjectStore<E, 5> store;
	{
		auto h1 = store.create("Hello1");
		CHECK(h1->message == "Hello1");
		CHECK(E::getCount() == 1);

		auto h2 = store.create("Hello2");
		auto h3 = store.create("Hello3");
		CHECK(E::getCount() == 3);
		{
			auto h4 = store.create("Hello4");
			auto h5 = store.create("Hello5");
			CHECK(E::getCount() == 5);
			CHECK(h5->message == "Hello5");
		}
		CHECK(E::getCount() == 3);

		auto h6 = store.create("Hello6");
		auto h7 = store.create("Hello7");

		CHECK(h6->message == "Hello6");
		CHECK(E::getCount() == 5);
	}
	CHECK(E::getCount() == 0);
}

struct TagTest2 {};
TEST_CASE("Assignments", "[refcounting]") {

	using E = MyType<TagTest2>;
	sos::SharedObjectStore<E, 5> store;
	{
		auto h1 = store.create("Hello1");
		CHECK(h1->message == "Hello1");
		CHECK(E::getCount() == 1);

		auto h2 = store.create("Hello2");
		auto h3 = store.create("Hello3");
		CHECK(E::getCount() == 3);

		h2 = h3;
		CHECK(E::getCount() == 2);

		CHECK(h2->message == h3->message);
		h2 = h1;
		CHECK(h2->message != h3->message);
		CHECK(E::getCount() == 2);

		h1 = store.create("Temp1");
		CHECK(E::getCount() == 3);
		h1 = std::move(h3);
		CHECK(E::getCount() == 2);

	}
	CHECK(E::getCount() == 0);
}

struct TagTest3 {};
TEST_CASE("Container", "[refcounting]") {

	using E = MyType<TagTest3>;
	sos::SharedObjectStore<E, 45> store;
	std::vector<sos::Handle<E>> handles;
	for(int i = 0; i < 21; i+=2) {
		handles.push_back(store.create(std::to_string(i)));
		CHECK(handles.back()->message == std::to_string(i));
	}
	for (int i = 21; i > 0; i -= 2) {
		handles.push_back(store.create(std::to_string(i)));
		CHECK(handles.back()->message == std::to_string(i));
	}
	std::sort(
		handles.begin(),
		handles.end(),
		[](auto&& l, auto&& r) {
			return std::stoll(l->message) < std::stoll(r->message);
		}
	);
	for (int i = 0; i < 21; i++) {
		CHECK(handles[i]->message == std::to_string(i));
	}

}

