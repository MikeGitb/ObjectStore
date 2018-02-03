cmake_minimum_required(VERSION 3.5)

project(SharedObjectStore)

add_library(sos INTERFACE)
add_library(Sos::sos ALIAS sos)
target_include_directories(sos INTERFACE include)
target_compile_features(sos INTERFACE cxx_std_17)


add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)


