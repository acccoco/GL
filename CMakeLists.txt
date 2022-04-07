cmake_minimum_required(VERSION 3.19)
project(GL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# config
configure_file(${CMAKE_SOURCE_DIR}/config.hpp ${CMAKE_BINARY_DIR}/config.hpp)

# system header and lib
if (APPLE)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif ()

# project include
include_directories(core ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})

# lib: glad
add_library(GLAD STATIC libs/glad/src/glad.c)
target_include_directories(GLAD PUBLIC libs/glad/include)

# lib: stb_image
add_library(STB STATIC libs/stb_image/src/stb_image.cpp)
target_include_directories(STB PUBLIC libs/stb_image/include)

# lib: imgui
#find_library(COCOA_LIB Cocoa)
#find_library(IOKit_LIB IOKit)
#find_library(CoreVideo_LIB CoreVideo)
add_library(IMGUI STATIC
        libs/imgui/imgui.cpp
        libs/imgui/imgui_demo.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_tables.cpp
        libs/imgui/imgui_widgets.cpp
        libs/imgui/backends/imgui_impl_glfw.cpp
        libs/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(IMGUI PUBLIC libs/imgui libs/imgui/backends)

# other libs
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

set(LIB_LINKS
        glfw
        GLAD
        STB
        IMGUI
        glm::glm
        ${ASSIMP_LIBRARIES}
        ${OPENGL_LIBRARIES})

# all targes
file(GLOB ALL_EXAMPLES RELATIVE ${CMAKE_SOURCE_DIR}/examples ${CMAKE_SOURCE_DIR}/examples/*)
foreach (EXAMPLE ${ALL_EXAMPLES})
    add_executable(${EXAMPLE} ${CMAKE_SOURCE_DIR}/examples/${EXAMPLE}/main.cpp)
    target_link_libraries(${EXAMPLE} ${LIB_LINKS})
endforeach ()
