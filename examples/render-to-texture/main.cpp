// https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/

#include <iostream>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/misc.h"
#include "core/model.h"
#include "core/texture.h"
#include "function/skybox/skybox.h"
#include "function/tex2d-visual/tex2d-visual.h"

#include "shader/lambert/lambert.h"


struct ThisFrameBuffer {
    GLuint frame_buffer{};
    GLuint depth_buffer;
    GLuint tex_color;
    const GLsizei WIDTH = 1024;
    const GLsizei HEIGHT = 1024;

    ThisFrameBuffer()
    {
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        depth_buffer = create_depth_buffer(WIDTH, HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

        tex_color = create_tex(WIDTH, HEIGHT);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_color, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "frame buffer uncomplete." << std::endl;
    }
};

class EngineTest : public Engine
{
    ThisFrameBuffer framebuffer;

    ShaderLambert shader_lambert;

    std::vector<Model> three_objs = Model::load_obj(MODEL_THREE_OBJS);

    SkyBox skybox;
    Tex2DVisual tex_visual;

    void init() override
    {
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
        glDepthFunc(GL_LEQUAL);

        shader_lambert.init(Camera::proj_matrix());
    }

    void tick_pre_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.frame_buffer);
        glViewport(0, 0, framebuffer.WIDTH, framebuffer.HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.draw_default_sky(camera.view_matrix(), Camera::proj_matrix());

        shader_lambert.update_per_fame(camera.view_matrix());
        for (auto &m: three_objs)
            shader_lambert.draw(m);
    }

    void tick_render() override {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport_(0, 0, window.width, window.height);

        tex_visual.show(framebuffer.tex_color);
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);
        ImGui::End();
    }
};

int main()
{
    auto engine = EngineTest();
    engine.engine_main();
}