#pragma once

#include "core/camera.h"
#include "core/env.h"
#include "core/shader.h"


class Engine
{
protected:
    struct {
        GLFWwindow *ptr = nullptr;

        int width  = 1600;
        int height = 1600;
    } window;

    Camera       camera;
    virtual void init() {}
    virtual void tick_logic() {}
    virtual void tick_gui() {}
    virtual void tick_pre_render() {}
    virtual void tick_render() {}


public:
    Engine()
    {
        spdlog_init();
        glfw_init();

        /// create window\n
        /// 与 glfw 相关的操作，提供的尺寸是真实尺寸（frambuffer）的一半
        window.ptr = glfwCreateWindow(window.width / 2, window.height / 2, "RTR", nullptr, nullptr);
        if (!window.ptr)
            std::cout << "fail to create window." << std::endl;
        glfwMakeContextCurrent(window.ptr);

        glad_init();

        // window callback
        glfwSetCursorPosCallback(window.ptr, mouse_pos_callback);
        glfwSetMouseButtonCallback(window.ptr, mouse_button_callback);
        glfwSetFramebufferSizeCallback(window.ptr, [](GLFWwindow *, int width, int height) {
            SPDLOG_INFO("width: {}, height: {}", width, height);
        });

        imgui_init(window.ptr);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    }

    void set_window_size(int width, int height)
    {
        this->window.width  = width;
        this->window.height = height;

#ifdef __APPLE__
        /// glfw 相关的操作，窗口尺寸是实际尺寸（和 framebuffer 尺寸相同）的一半
        glfwSetWindowSize(window.ptr, width / 2, height / 2);
#elif
        glfwSetWindowSize(window.ptr, width, height);
#endif
    }

    // tick order
    //   tick logic -> tick camera -> tick gui -> tick render
    void engine_main()
    {
        try
        {
            init();

            glEnable(GL_DEPTH_TEST);
            while (!glfwWindowShouldClose(window.ptr))
            {
                // tick logic
                glfwPollEvents();
                check_close_window(window.ptr);
                tick_logic();

                // tick camera
                camera.update_position(window.ptr);
                camera.update_dir_eular();

                // tick gui
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                tick_gui();
                ImGui::Render();

                // tick render
                tick_pre_render();
                tick_render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(window.ptr);
            }
            glfwDestroyWindow(window.ptr);
            glfwTerminate();
        } catch (std::exception &e)
        {
            SPDLOG_ERROR("exception occurs, exit.");
        }
    }
};
