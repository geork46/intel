project(Test)
cmake_minimum_required(VERSION 2.8)

file(GLOB SRC_LIST "./src/*.cpp")
file(GLOB HDR_LIST "./include/*.h")


include_directories("./include/")
add_definitions(-ansi -std=c++0x)

add_executable(${PROJECT_NAME} ${HDR_LIST} ${SRC_LIST})

