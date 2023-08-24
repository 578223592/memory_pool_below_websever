cmake_minimum_required(VERSION 3.22)
project(memory_pool)

set(CMAKE_CXX_STANDARD 11)

# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 添加头文件搜索路径
include_directories(include)

add_executable(memory_pool_test main.cpp
        mempory_pool_mutex.h mempory_pool_mutex.cpp)
