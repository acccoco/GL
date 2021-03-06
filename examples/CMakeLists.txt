# 默认 examples 内是双层的，即：
# examples
# |_ <major-example>
#    |_ <minor-example>
#       |_ main.cpp
#       |_ shader


# 利用 m4 预处理 glsl 代码
function(preprocess_shader source_dir target_dir out_shaders)
    # 检查一下全局变量是否有效
    if (${RT_SHADER_INCLUDE} STREQUAL "")
        message(FATAL_ERROR "variable RT_SHADER_INCLUDE is null string")
    endif()

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


# 遍历所有的文件夹，将所有的 minor 层级添加为 target
file(GLOB major_path_list ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach (major_path ${major_path_list})
    # 排除非文件夹
    if (NOT IS_DIRECTORY ${major_path})
        continue()
    endif ()

    get_filename_component(major_name ${major_path} NAME)

    file(GLOB minor_path_list ${major_path}/*)
    foreach (minor_path ${minor_path_list})
        # 排除非文件夹
        if (NOT IS_DIRECTORY ${minor_path})
            continue()
        endif ()

        # 注入变量到配置文件中
        # EXAMPLE_CUR_PATH 这个变量是给 cpp 文件使用的
        set(EXAMPLE_CUR_PATH ${minor_path})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.in.h ${minor_path}/config.hpp)

        # 处理所有的 shader
        preprocess_shader(${minor_path} ${minor_path}/shader shader_list)

        # 确定 target 的名称
        get_filename_component(minor_name ${minor_path} NAME)
        set(target_name "${major_name}.${minor_name}")

        # build target
        add_executable(${target_name} ${minor_path}/main.cpp ${shader_list})
        target_link_libraries(${target_name} frame)
    endforeach ()
endforeach ()
