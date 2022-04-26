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
#include "shader/blinn-phong/blinn-phong.h"


struct CubeFramebuffer {
    GLuint frame_buffer{};
    GLuint depth_buffer;
    GLuint filtered_env_map;
    const GLsizei size = 1024;

    CubeFramebuffer()
    {
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        depth_buffer = create_depth_buffer(size, size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

        filtered_env_map = create_cube_map(size);
    }
};


class EngineTest : public Engine
{
    std::vector<Model> model_three = Model::load_obj(MODEL_THREE_OBJS);
    std::vector<Model> model_202 = Model::load_obj(MODEL_202_CHAN);

    ShaderLambert shader_lambert;
    ShaderBlinnPhong shader_phong;

    CubeFramebuffer buffer;

    SkyBox sky_box;
    CubeMapVisual cube_visual;
    Tex2DVisual tex2d_visua;

    void init() override {
        glDepthFunc(GL_LEQUAL);
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);

        shader_phong.init(Camera::proj_matrix());
    }

    void tick_pre_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer.frame_buffer);
        glViewport(0, 0, buffer.size, buffer.size);

        glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 100.f);
        shader_lambert.init(proj);

        auto draw_dir = [&](GLenum textarget, const glm::vec3 &front, const glm::vec3 &up) {
            // textarget: for cube map, specify which face is to be attached
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textarget, buffer.filtered_env_map, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 m_view = glm::lookAt(camera.get_pos(), camera.get_pos() + front, up);

            sky_box.draw_default_sky(m_view, proj);

            shader_lambert.update_per_fame(m_view);
            for (auto &m: model_three)
                shader_lambert.draw(m);
        };

        // for historical reason of cubemap, reverse UP direction of camera.
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_X, POSITIVE_X, -POSITIVE_Y);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, NEGATIVE_X, -POSITIVE_Y);
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, POSITIVE_Z, -POSITIVE_Y);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, NEGATIVE_Z, -POSITIVE_Y);  // front
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, POSITIVE_Y, POSITIVE_Z);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, NEGATIVE_Y, NEGATIVE_Z);   // down
    }

    void tick_render() override {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport_(0, 0, window.width, window.height);

        cube_visual.draw_as_skybox(camera.view_matrix(), Camera::proj_matrix(), buffer.filtered_env_map);

        shader_lambert.update_per_fame(Camera::proj_matrix());
        shader_lambert.update_per_fame(camera.view_matrix());
        for (auto & m: model_202)
            shader_lambert.draw(m);
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