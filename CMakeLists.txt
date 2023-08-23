cmake_minimum_required(VERSION 3.22)
project(memory_pool)

set(CMAKE_CXX_STANDARD 11)

add_executable(memory_pool_test main.cpp
        mempory_pool_mutex.h mempory_pool_mutex.cpp)
