# lib: glad
add_library(GLAD STATIC glad/src/glad.c)
target_include_directories(GLAD PUBLIC glad/include)

# lib: stb_image
#add_library(STB STATIC stb_image/src/stb_image.cpp)
#target_include_directories(STB PUBLIC stb_image/include)

# lib: imgui
add_library(IMGUI STATIC
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(IMGUI
        PUBLIC imgui imgui/backends
        PRIVATE /opt/homebrew/include)

