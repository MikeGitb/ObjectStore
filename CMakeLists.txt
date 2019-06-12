cmake_minimum_required(VERSION 3.5)

option(SOS_INCLUDE_TESTS "Include small object store tests" OFF)
option(SOS_INCLUDE_EXAMPLES "Include small object store tests" OFF)

set(Sos_VERSION 0.1)

project(SharedObjectStore)

add_library(sos INTERFACE)
add_library(Sos::sos ALIAS sos)
target_include_directories(sos INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)
target_compile_features(sos INTERFACE cxx_std_17)

if(SOS_INCLUDE_EXAMPLES)
	add_subdirectory(examples)
endif()

if(SOS_INCLUDE_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

include(cmake/Install.cmake)

