cmake_minimum_required(VERSION 3.9)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#find_package(Catch REQUIRED)
add_executable(sos-tests
	main.cpp
	test_existance.cpp
	test_refcounting.cpp)
target_link_libraries(sos-tests PRIVATE Sos::sos)
target_include_directories(sos-tests PUBLIC libs)

if (MSVC)
	STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
	#deactivate noisy messages on msvc Wall
	target_compile_options(sos-tests PUBLIC /permissive- /wd4710 )
	target_compile_definitions(sos-tests PUBLIC /D_SCL_SECURE_NO_WARNINGS /DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)

else()
	target_compile_options(sos-tests PUBLIC -Wall -Wextra)
endif()


include(libs/catch2/ParseAndAddCatchTests.cmake)
ParseAndAddCatchTests(sos-tests)

