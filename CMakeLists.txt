cmake_minimum_required(VERSION 3.5)

project(SharedObjectStore)

set(CMAKE_CXX_STANDARD 14)

add_library(sos INTERFACE)
add_library(Sos::sos ALIAS sos)
target_include_directories(sos INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
#target_compile_features(sos INTERFACE cxx_std_17)


add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)


