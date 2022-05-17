cmake_minimum_required(VERSION 3.19)
project(GL)


# compile setting
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


# dir config
set(GL_MODEL_DIR "${CMAKE_SOURCE_DIR}/model")
set(GL_TEXTURE_DIR "${CMAKE_SOURCE_DIR}/texture")
set(GL_SHADER_DIR "${CMAKE_SOURCE_DIR}/shader")
configure_file(${CMAKE_SOURCE_DIR}/config.hpp ${CMAKE_BINARY_DIR}/config.hpp)


# system header and lib
if (APPLE)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif ()


# lib: glad
add_library(GLAD STATIC ext/glad/src/glad.c)
target_include_directories(GLAD PUBLIC ext/glad/include)

# lib: stb_image
add_library(STB STATIC ext/stb_image/src/stb_image.cpp)
target_include_directories(STB PUBLIC ext/stb_image/include)

# lib: imgui
#find_library(COCOA_LIB Cocoa)
#find_library(IOKit_LIB IOKit)
#find_library(CoreVideo_LIB CoreVideo)
add_library(IMGUI STATIC
        ext/imgui/imgui.cpp
        ext/imgui/imgui_demo.cpp
        ext/imgui/imgui_draw.cpp
        ext/imgui/imgui_tables.cpp
        ext/imgui/imgui_widgets.cpp
        ext/imgui/backends/imgui_impl_glfw.cpp
        ext/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(IMGUI PUBLIC ext/imgui ext/imgui/backends)

# other ext
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)


# 链接需要用到的库
set(LIB_LINKS
        glfw
        spdlog::spdlog
        fmt::fmt
        GLAD
        STB
        IMGUI
        glm::glm
        ${ASSIMP_LIBRARIES}
        ${OPENGL_LIBRARIES})


# project include
include_directories(core ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})


# all targes
file(GLOB all_examples RELATIVE ${CMAKE_SOURCE_DIR}/examples ${CMAKE_SOURCE_DIR}/examples/*)
foreach (example_name ${all_examples})

    set(example_path ${CMAKE_SOURCE_DIR}/examples/${example_name})
    set(example_shader_path ${CMAKE_SOURCE_DIR}/examples/${example_name}/shader)

    if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/examples/${example_name})

        # 编译链接的目标
        add_executable(${example_name} ${example_path}/main.cpp)
        target_link_libraries(${example_name} ${LIB_LINKS})

        # 确保 examples/xxx/shader 文件夹存在
        if ((NOT EXISTS ${example_shader_path}) OR (NOT IS_DIRECTORY ${example_shader_path}))
            file(MAKE_DIRECTORY ${example_shader_path})
        endif ()

        # 利用 m4 预处理 glsl 代码
        file(GLOB shader_name_list RELATIVE ${example_path} ${example_path}/*.vert ${example_path}/*.frag)
        foreach (shader_name ${shader_name_list})
            add_custom_command(TARGET ${example_name}
                    POST_BUILD  # 在 build 之后允许该命令
                    COMMAND m4 --include=${GL_SHADER_DIR} ${example_path}/${shader_name} > ${example_shader_path}/${shader_name}
                    COMMAND echo "preprocess ${shader_name}"
                    VERBATIM    # 允许 cmake 转义
                    )
        endforeach ()
    endif ()
endforeach ()
