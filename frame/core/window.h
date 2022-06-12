#pragma once

#include <array>
#include <vector>

#include <GLFW/glfw3.h>


struct Window {
    static inline const char *window_title = "RTR";

    /**
     * 初始化 glfw，创建窗口，设置 OpenGL 上下文，设置回调函数
     * @return 操作是否成功
     */
    static bool init();

    /**
     * 窗口关闭，释放资源
     */
    static void terminate();

    /**
     * 交换双缓冲
     */
    static void swap_framebuffer() { glfwSwapBuffers(_window); }

    /**
     * 是否应该关闭窗口
     */
    static bool should_close() { return glfwWindowShouldClose(_window); }

    /**
     * 改变窗口内的 framebuffer 的大小
     */
    static void set_framebuffer_size(int width, int height);

    /**
     * 清除之前缓存的按键状态，处理新的按键
     */
    static void tick_window_event();

    static GLFWwindow *window() { return _window; }

    static int framebuffer_width() { return _framebuffer_width; }
    static int framebuffer_height() { return _framebuffer_height; }

    /**
     * 获得当前的鼠标位置，左上角是 (0, 0)，鼠标范围基于 window 的尺寸
     * @note 在 APPLE 平台，window 尺寸是 framebuffer 的 1/2
     */
    static std::array<double, 2> cursor_pos() { return {_cursor_pos_x, _cursor_pos_y}; }

    /**
     * 当前帧键盘按键是否发生了某个动作
     * @param key 鼠标或键盘的按键，例如 GLFW_KEY_ESC
     * @param action 按键动作，例如 GLFW_PRESS, GLFW_RELEASE
     */
    static bool key_has_action(int key, int action);

    /**
     * 键盘按键的最后一个动作，也就是按键当前的状态
     * @return 只能是 GLFW_PRESS, GLFW_RELEASE
     */
    static int keyboard_last_action(int key) { return glfwGetKey(_window, key); }

    /**
     * 鼠标按键的最后一个动作，也就是按键当前的状态
     * @return 只能是 GLFW_PRESS, GLFW_RELEASE
     */
    static int mouse_button_last_action(int button) { return glfwGetMouseButton(_window, button); }


private:
    static inline GLFWwindow *_window;

    static inline int _framebuffer_width  = 1600;
    static inline int _framebuffer_height = 1600;

    static inline double _cursor_pos_x;
    static inline double _cursor_pos_y;

    /**
     * 当前帧按键的动作
     * 格式为pair(key, action) key 包括鼠标按键和键盘按键，action 可以是 GLFW_PRESS, GLFW_RELEASE 等
     */
    static inline std::vector<std::pair<int, int>> _current_key_actions;

    /**
     * 鼠标位置改变的回调。左上角是 (0, 0)，鼠标位置基于 window 的尺寸
     * @note 在 APPLE 中，framebuffer 是 window 大小的 2 倍
     */
    static void callback_cursor_pos(GLFWwindow *, double xpos, double ypos);

    /**
     * 鼠标按键操作的回调函数。按下和松开都会触发这个回调函数
     * @param button 鼠标按键
     * @param action 只能是 GLFW_PRESS 或者 GLFW_RELEASE
     */
    static void callback_mouse_button(GLFWwindow *, int button, int action, int);

    /**
     * 通过 glfwSetWindowSize 等操作改变窗口大小时，也会改变 framebuffer 的尺寸，会同步触发这个回调函数
     */
    static void callback_framebuffer_size(GLFWwindow *, int width, int height);

    /**
     * 键盘按键操作的回调
     * @param key 键盘的按键
     * @param action 按键的动作，只能是：GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
     * @note 关于 GLFW_REPEAT 例如，日常使用时，一直按住 A 键，输入框中就会出现重复的很多个字符 A\n
     *       操作系统中可以设置要按多久才会出现重复的动作\n
     *       按下后的动作为：{PRESS - REPEAT - REPEAT - ... - REPEAT - RELEASE}
     */
    static void callback_keyboard(GLFWwindow *, int key, int, int action, int);
};