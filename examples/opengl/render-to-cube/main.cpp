/**
 * 第一遍：将场景会知道 cube-map 上
 * 第二遍：将 cube-map 作为天空盒显示出来
 */

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/misc.h"
#include "core/texture.h"


#include "shader/skybox/skybox.h"
#include "shader/tex2d-visual/tex-visual.h"
#include "shader/diffuse/diffuse.h"
#include "shader/blinn-phong/blinn-phong.h"


struct CubeFramebuffer {
    GLuint        frame_buffer{};
    GLuint        depth_buffer;
    GLuint        filtered_env_map;
    const GLsizei size = 1024;

    CubeFramebuffer()
    {
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        depth_buffer = create_depth_buffer(size, size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  depth_buffer);

        filtered_env_map = new_cubemap({
                .size            = size,
                .internal_format = GL_RGB32F,
                .external_format = GL_RGB,
                .external_type   = GL_FLOAT,
        });
    }
};


class EngineTest : public Engine
{
    std::vector<RTObject> model_three = ImportObj::load_obj(MODEL_THREE_OBJS);
    std::vector<RTObject> model_202   = ImportObj::load_obj(MODEL_202_CHAN);

    ShaderDiffuse    shader_lambert;
    ShaderBlinnPhong shader_phong;
    ShaderTexVisual  shader_texvisual;

    CubeFramebuffer buffer;

    SkyBox        sky_box;
    CubeMapVisual cube_visual;

    void init() override
    {
        glDepthFunc(GL_LEQUAL);
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);

        shader_phong.init(camera.proj_matrix());
    }

    void tick_pre_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer.frame_buffer);
        glViewport(0, 0, buffer.size, buffer.size);

        glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 100.f);
        shader_lambert.init(proj);

        auto draw_dir = [&](GLenum textarget, const glm::vec3 &front, const glm::vec3 &up) {
            // textarget: for cube map, specify which face is to be attached
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textarget,
                                   buffer.filtered_env_map, 0);
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
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, NEGATIVE_Z, -POSITIVE_Y);    // front
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, POSITIVE_Y, POSITIVE_Z);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, NEGATIVE_Y, NEGATIVE_Z);    // down
    }

    void tick_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, Window::framebuffer_width(), Window::framebuffer_height());

        cube_visual.draw_as_skybox(camera.view_matrix(), camera.proj_matrix(),
                                   buffer.filtered_env_map);

        shader_lambert.update_per_fame(camera.proj_matrix());
        shader_lambert.update_per_fame(camera.view_matrix());
        for (auto &m: model_202)
            shader_lambert.draw(m);
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y,
                    camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw,
                    camera.get_euler().pitch);
        ImGui::End();
    }
};


int main()
{
    auto engine = EngineTest();
    engine.engine_main();
}