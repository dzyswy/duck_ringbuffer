cmake_minimum_required(VERSION 3.8)
project(duck_ringbuffer)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package (glog REQUIRED)

include_directories(./)

file(GLOB_RECURSE SOURCES "pipe/*.cpp" "thread/*.cpp")

add_executable(main ${SOURCES} main.cpp)
target_link_libraries (main glog::glog)
 












