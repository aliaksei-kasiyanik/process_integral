cmake_minimum_required(VERSION 3.6)
project(process_integral)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(process_integral ${SOURCE_FILES})