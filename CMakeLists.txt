project(Test)
cmake_minimum_required(VERSION 2.8)

file(GLOB SRC_LIST "./src/*.cpp")
file(GLOB HDR_LIST "./include/*.h")

set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_COMPILER_ID Intel)
set (CMAKE_CXX_COMPILER icpc)

include_directories("./include/")
include_directories("/opt/intel/include")
add_definitions(-ansi -std=c++11)

add_executable(${PROJECT_NAME} ${HDR_LIST} ${SRC_LIST})
