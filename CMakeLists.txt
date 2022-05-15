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

# all targes
file(GLOB ALL_EXAMPLES RELATIVE ${CMAKE_SOURCE_DIR}/examples ${CMAKE_SOURCE_DIR}/examples/*)
foreach (EXAMPLE ${ALL_EXAMPLES})
    if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/examples/${EXAMPLE})
        add_executable(${EXAMPLE} ${CMAKE_SOURCE_DIR}/examples/${EXAMPLE}/main.cpp)
        target_link_libraries(${EXAMPLE} ${LIB_LINKS})
    endif()
endforeach ()
