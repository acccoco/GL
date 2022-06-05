/**
 * 第一遍：将场景绘制在 texture-2D 上
 * 第二遍：将 texture-2D 绘制在一个正方形上。
 * 注：可以进一步在这个正方形生进行后处理，比如模糊什么的。
 * 参考：https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
 */


#include <iostream>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/misc.h"
#include "core/model.h"
#include "core/texture.h"

#include "shader/skybox/skybox.h"
#include "shader/tex2d-visual/tex-visual.h"
#include "shader/diffuse/diffuse.h"


struct ThisFrameBuffer {
    GLuint        frame_buffer{};
    GLuint        depth_buffer;
    GLuint        tex_color;
    const GLsizei WIDTH  = 1024;
    const GLsizei HEIGHT = 1024;

    ThisFrameBuffer()
    {
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        depth_buffer = create_depth_buffer(WIDTH, HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

        tex_color = new_tex2d({
                .width           = WIDTH,
                .height          = HEIGHT,
                .internal_format = GL_RGB32F,
                .external_format = GL_RGB,
                .external_type   = GL_FLOAT,
        });
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_color, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "frame buffer uncomplete." << std::endl;
    }
};

class EngineTest : public Engine
{
    ThisFrameBuffer framebuffer;

    ShaderDiffuse   shader_lambert;
    ShaderTexVisual shader_texvisual;

    std::vector<Model> three_objs   = Model::load_obj(MODEL_THREE_OBJS);
    Model              model_square = Model::load_obj(MODEL_SQUARE)[0];

    SkyBox skybox;

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

    void tick_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport_(0, 0, window.width, window.height);

        shader_texvisual.draw(model_square, framebuffer.tex_color);
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