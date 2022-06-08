#pragma once

#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


inline void glfw_error_callback(int error, const char *str)
{
    std::fprintf(stderr, "glfw error(%d): %s\n", error, str);
}


inline void glfw_init()
{
    // init glfw
    glfwSetErrorCallback(glfw_error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

inline void spdlog_init()
{
    spdlog::set_level(spdlog::level::debug);

    /**
     * %L 日志级别，缩写
     * %t 线程
     *
     * 以下选项必须要用 SPDLOG_INFO 才能生效
     * %s 文件的 basename
     * %# 行号
     * %! 函数名
     *
     * 格式控制：
     * %-4!<flag> 表示左对齐，4位，超出截断
     */
    spdlog::set_pattern(/*"[%H:%M:%S]"*/ "[%^%L%$][%10!s:%-3!#] %v");
}

inline void glad_init()
{
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
        std::fprintf(stderr, "fail to environment_init glad.");
}

inline void imgui_init(GLFWwindow *window)
{
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

inline void check_close_window(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
