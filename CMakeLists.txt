cmake_minimum_required(VERSION 3.20)

project(Cache_system)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

aux_source_directory(. SRC_LIST)

# 配置最终的可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件的搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

# 生成可执行文件
add_executable(Cache_system ${SRC_LIST})

