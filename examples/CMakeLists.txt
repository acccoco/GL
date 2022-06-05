# 默认 examples 内是双层的，即：
# examples
# |_ <major-example>
#    |_ <minor-example>
#       |_ main.cpp
#       |_ shader


file(GLOB major_path_list ${RT_EXAMPLES_DIR}/*)
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
        set(target_name ${major_name}.${minor_name})

        # build target
        add_executable(${target_name} ${minor_path}/main.cpp ${shader_list})
        target_link_libraries(${target_name} frame)
    endforeach ()
endforeach ()
