#include <sos/sos.h>

#include <catch2/catch.hpp>

#include <string>

using namespace mgb;

TEST_CASE("get name", "[General]") {
	REQUIRE(sos::my_name() == std::string("Shared Object Store Library"));
}