#pragma once

#include "./camera.h"
#include "./ext-init.h"
#include "./shader.h"
#include "./opengl-misc.h"


class Engine
{
protected:
    Camera2 camera;

    /**
     * 每个项目自己的初始化函数
     */
    virtual void init() {}

    virtual void tick_gui() {}
    virtual void tick_pre_render() {}
    virtual void tick_render() {}


public:
    /**
     * 需要先初始化一些内容，为项目提供环境
     */
    Engine()
    {
        spdlog_init();
        if (!Window::init())
        {
            std::fprintf(stderr, "fail to init window.\n");
            exit(0);
        }
        imgui_init(Window::window());
        glad_init();
    }


    void engine_main()
    {
        try
        {
            init();
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            while (!Window::should_close())
                main_loop();
            Window::terminate();
        } catch (std::exception &e)
        {
            SPDLOG_ERROR("exception occurs, exit.");
        }
    }

private:
    void main_loop()
    {
        // tick logic
        Window::tick_window_event();

        // tick camera
        camera.tick_rotate();
        camera.tick_move();

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

        Window::swap_framebuffer();
    }
};
