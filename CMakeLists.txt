cmake_minimum_required(VERSION 3.5)

set(Sos_VERSION 0.1)

project(SharedObjectStore)

add_library(sos INTERFACE)
add_library(Sos::sos ALIAS sos)
target_include_directories(sos INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)
target_compile_features(sos INTERFACE cxx_std_17)


add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)

include(cmake/Install.cmake)

