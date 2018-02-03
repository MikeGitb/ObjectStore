cmake_minimum_required(VERSION 3.5)


#find_package(Catch REQUIRED)
add_executable(sos-tests
	main.cpp
	test_existance.cpp
	test_refcounting.cpp)
target_link_libraries(sos-tests PRIVATE Sos::sos)
target_include_directories(sos-tests PUBLIC libs)

include(libs/catch2/Catch.cmake)

catch_discover_tests(sos-tests)

