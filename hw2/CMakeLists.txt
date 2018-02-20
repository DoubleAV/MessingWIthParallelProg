cmake_minimum_required(VERSION 2.8)
project(hw2)

set(CMAKE_CXX_STANDARD 11)
SET(GCC_EXE_LINKER_FLAGS    "-static")

add_executable(hw2 main.cpp)