# 注入配置文件
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shader)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/frame-config.in.h ${CMAKE_CURRENT_SOURCE_DIR}/frame-config.hpp)


# 手动编译的第三方库
add_subdirectory(ext)


# 通过命令行安装的第三方库
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(TinyGLTF REQUIRED HINTS ./ext/tinyGLTF)
if (APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(IOKIT_LIBRARY IOKit)
endif ()


# 链接需要用到的库
set(LIB_LINKS
        glfw
        spdlog::spdlog
        fmt::fmt
        GLAD
        # STB
        TinyGLTF
        IMGUI
        glm::glm
        ${ASSIMP_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${COCOA_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${IOKIT_LIBRARY})


file(GLOB all_cpps ${CMAKE_CURRENT_SOURCE_DIR}/core/src/*.cpp)
add_library(frame STATIC ${all_cpps})
target_compile_definitions(frame PRIVATE FRAME)


if (APPLE)
    target_include_directories(frame PUBLIC /opt/homebrew/include)
    target_link_directories(frame PUBLIC /opt/homebrew/lib)
endif ()
target_include_directories(frame PUBLIC ./ ${TinyGLTF_INCLUDE_DIR})
target_link_libraries(frame PUBLIC ${LIB_LINKS})

