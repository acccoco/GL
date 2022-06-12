#include "../window.h"

#include <cstdio>
#include <algorithm>


bool Window::init()
{
    /// init glfw
    glfwSetErrorCallback([](int error, const char *str) {
        std::fprintf(stderr, "glfw error(%d): %s\n", error, str);
    });
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    /// 创建窗口
#ifdef __APPLE__
    /// 与 glfw 相关的操作，window 尺寸是 framebuffer 尺寸的 1/2
    _window = glfwCreateWindow(_framebuffer_width / 2, _framebuffer_height / 2, window_title,
                               nullptr, nullptr);
#else
    window = glfwCreateWindow(_window_width, _window_height, window_title, nullptr, nullptr);
#endif
    if (_window == nullptr)
        return false;

    /// 设置 OpenGL 上下文
    glfwMakeContextCurrent(_window);

    /// callback
    glfwSetCursorPosCallback(_window, callback_cursor_pos);
    glfwSetMouseButtonCallback(_window, callback_mouse_button);
    glfwSetFramebufferSizeCallback(_window, callback_framebuffer_size);
    glfwSetKeyCallback(_window, callback_keyboard);

    return true;
}


void Window::terminate()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}


void Window::set_framebuffer_size(int width, int height)
{
    _framebuffer_width  = width;
    _framebuffer_height = height;

#ifdef __APPLE__
    /// 与 glfw 相关的操作，window 尺寸是 framebuffer 尺寸的 1/2
    glfwSetWindowSize(_window, width / 2, height / 2);
#elif
    glfwSetWindowSize(window.ptr, width, height);
#endif
}


void Window::tick_window_event()
{
    _current_key_actions.clear();

    /// 读取「事件队列」，触发回调函数
    glfwPollEvents();

    /// 检查是否需要关闭窗口
    if (key_has_action(GLFW_KEY_ESCAPE, GLFW_PRESS))
        glfwSetWindowShouldClose(_window, true);
}


bool Window::key_has_action(int key, int action)
{
    return std::any_of(_current_key_actions.begin(), _current_key_actions.end(),
                       [key, action](const std::pair<int, int> &p) {
                           return (p.first == key) && (p.second == action);
                       });
}


void Window::callback_cursor_pos(GLFWwindow *, double xpos, double ypos)
{
    _cursor_pos_x = xpos;
    _cursor_pos_y = ypos;
}


void Window::callback_mouse_button(GLFWwindow *, int button, int action, int)
{
    _current_key_actions.emplace_back(button, action);
}


void Window::callback_framebuffer_size(GLFWwindow *, int width, int height)
{
    std::fprintf(stdout, "framebuffer size: %d, %d\n", width, height);
}


void Window::callback_keyboard(GLFWwindow *, int key, int, int action, int)
{
    _current_key_actions.emplace_back(key, action);
}
