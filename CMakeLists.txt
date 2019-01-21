#[[
cmake_minimum_required (VERSION 2.8)
include_directories(/usr/include/mpi/)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
project (Ping-pong)
set(SOURCES main.cpp header.h)
add_executable(Ping-pong ${SOURCES})
]]

set(CMAKE_C_COMPILER mpicc)
set(CMAKE_CXX_COMPILER mpicxx)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#target_link_libraries(main mpi)

set(SOURCE_FILES main.cpp)

add_executable(ping-pong ${SOURCE_FILES})