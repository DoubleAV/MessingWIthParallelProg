cmake_minimum_required(VERSION 2.8)
project(hw3)

set(CMAKE_CXX_STANDARD 11)

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2 -std=gnu++0x")
endif()

add_executable(hw3 main.cpp)

TARGET_LINK_LIBRARIES(hw3 pthread)
