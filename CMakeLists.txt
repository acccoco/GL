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


# 利用 m4 预处理 glsl 代码
function(preprocess_shader source_dir target_dir out_shaders)
    # 确保目标文件夹存在
    if ((NOT EXISTS ${target_dir}) OR (NOT IS_DIRECTORY ${target_dir}))
        file(MAKE_DIRECTORY ${target_dir})
    endif ()

    # 生成所有的预处理规则
    file(GLOB shader_name_list RELATIVE ${source_dir} ${source_dir}/*.vert ${source_dir}/*.frag)
    foreach (shader_name ${shader_name_list})
        add_custom_command(
                OUTPUT ${target_dir}/${shader_name}
                COMMAND m4 --include=${RT_SHADER_INCLUDE} ${source_dir}/${shader_name} > ${target_dir}/${shader_name}
                DEPENDS ${source_dir}/${shader_name}
                VERBATIM        # 允许 cmake 转义
        )
    endforeach ()

    # 输出 shader 列表
    set(shader_path_list ${shader_name_list})
    list(TRANSFORM shader_path_list PREPEND "${target_dir}/")
    set(${out_shaders} ${shader_path_list} PARENT_SCOPE)    # 要暴露变量，因此使用这种语法
endfunction()


# 添加所有的 examples
add_subdirectory(examples)