cmake_minimum_required(VERSION 2.8)
project(hw2)

set(CMAKE_CXX_STANDARD 11)

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
endif()

add_executable(hw2 main.cpp)
