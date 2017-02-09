cmake_minimum_required(VERSION 3.5)
project(SBC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -Wall")

include_directories(includes)

set(SOURCE_FILES includes/Communication.h sources/Communication.cpp includes/EmBencode.h sources/EmBencode.cpp includes/Mcc.h sources/Mcc.cpp includes/AdminModule.h sources/AdminModule.cpp main.cpp)

add_executable(SBC ${SOURCE_FILES})