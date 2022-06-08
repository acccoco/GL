cmake_minimum_required(VERSION 3.19)
project(RT-Note)


# 编译选项
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


# 核心的框架
add_subdirectory(frame)


# 文件夹相关的配置
set(RT_MODEL_DIR ${CMAKE_SOURCE_DIR}/assets/model)
set(RT_TEXTURE_DIR ${CMAKE_SOURCE_DIR}/assets/texture)
set(RT_SHADER_INCLUDE ${CMAKE_SOURCE_DIR}/frame/shader)
set(RT_EXAMPLES_DIR ${CMAKE_SOURCE_DIR}/examples)


# 添加所有的 examples
add_subdirectory(examples)