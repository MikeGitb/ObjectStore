#include <sos/sos.h>

#include <catch2/catch.hpp>

#include <string>
#include <atomic>

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
		REQUIRE(h1->message == "Hello1");
		REQUIRE(E::getCount() == 1);

		auto h2 = store.create("Hello2");
		auto h3 = store.create("Hello3");
		REQUIRE(E::getCount() == 3);
		{
			auto h4 = store.create("Hello4");
			auto h5 = store.create("Hello5");
			REQUIRE(E::getCount() == 5);
			REQUIRE(h5->message == "Hello5");
		}
		REQUIRE(E::getCount() == 3);

		auto h6 = store.create("Hello6");
		auto h7 = store.create("Hello7");

		REQUIRE(h6->message == "Hello6");
		REQUIRE(E::getCount() == 5);
	}
	REQUIRE(E::getCount() == 0);
}
