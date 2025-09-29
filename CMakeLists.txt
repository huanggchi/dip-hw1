cmake_minimum_required(VERSION 3.10)
project(DIP_HW1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(hw1a src/hw1a.cpp)
add_executable(hw1b src/hw1b.cpp)
add_executable(hw1c src/hw1c.cpp)
