#include "core/window.h"
#include <cstdio>

int main() {
    Window::init();
    std::fprintf(stdout, "acc\n");
    while (!Window::should_close()) {
        Window::tick_window_event();

        if (Window::key_has_action(GLFW_KEY_W, GLFW_PRESS))
            std::printf("key press");
    }
    Window::terminate();
}
