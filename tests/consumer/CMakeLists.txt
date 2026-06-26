cmake_minimum_required(VERSION 3.20)
project(defer_consumer C)

find_package(defer CONFIG REQUIRED)

add_executable(defer_consumer main.c)
target_link_libraries(defer_consumer PRIVATE defer::defer)
