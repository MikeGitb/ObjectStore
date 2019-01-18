#include <sos/sos.h>

#include <catch2/catch.hpp>

#include <string>

using namespace mgb;

namespace {
struct TestStruct {
	int i;
};
} // namespace

TEST_CASE( "Compilation", "[sos][General]" )
{
	sos::SharedObjectStore<TestStruct, 100> store;

	auto mut_handle = store.create();

	[[maybe_unused]] auto i1 = static_cast<TestStruct&>( mut_handle );
	[[maybe_unused]] auto i2 = static_cast<const TestStruct&>( mut_handle );
	[[maybe_unused]] auto i3 = *mut_handle;
	[[maybe_unused]] auto i4 = mut_handle->i;

	auto handle = std::move( mut_handle ).lock();

	[[maybe_unused]] auto i5 = static_cast<const TestStruct&>( mut_handle );
	[[maybe_unused]] auto i6 = *mut_handle;
	[[maybe_unused]] auto i7 = mut_handle->i;
}