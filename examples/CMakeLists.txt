cmake_minimum_required(VERSION 3.5)

add_executable(hello-world hello_world.cpp)
target_link_libraries(hello-world PRIVATE Sos::sos)

add_executable(basic-use basic_use.cpp)
target_link_libraries(basic-use PRIVATE Sos::sos)

