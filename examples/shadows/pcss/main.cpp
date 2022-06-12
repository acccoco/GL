/**
 * 首先生成 2D 的 shadow map，然后绘制软/硬阴影
 * 手动在 pcss.frag 里面去更改阴影类型：shadow-mapping，pcf，pcss
 */


#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/misc.h"
#include "core/texture.h"

#include "shader/tex2d-visual/tex-visual.h"
#include "shader/diffuse/diffuse.h"
#include "shader/blinn-phong/blinn-phong.h"


struct DepthFramebuffer {
    GLuint        frame_buffer{};
    GLuint        depth_buffer;
    GLuint        shadow_map;
    const GLsizei size = 1024;

    DepthFramebuffer()
    {
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        depth_buffer = create_depth_buffer(size, size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  depth_buffer);

        shadow_map = new_tex2d({
                .width           = size,
                .height          = size,
                .internal_format = GL_RGB32F,
                .external_format = GL_RGB,
                .external_type   = GL_FLOAT,
        });
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadow_map, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            SPDLOG_ERROR("frame buffer uncomplete.");
    }
};


class TestEngine : public Engine
{
    DepthFramebuffer buffer;

    std::vector<RTObject> model_three_obj  = ImportObj::load_obj(MODEL_THREE_OBJS);
    std::vector<RTObject> model_matrix     = ImportObj::load_obj(MODEL_SPHERE_MATRIX);
    std::vector<RTObject> model_202        = ImportObj::load_obj(MODEL_202_CHAN);
    std::vector<RTObject> model_diona      = ImportObj::load_obj(MODEL_DIONA);
    RTObject              model_cube       = ImportObj::load_obj(MODEL_CUBE)[0];
    RTObject              model_light      = ImportObj::load_obj(MODEL_LIGHT)[0];
    RTObject              model_floor      = ImportObj::load_obj(MODLE_FLOOR)[0];
    RTObject              model_gray_floor = ImportObj::load_obj(MODEL_GRAY_FLOOR)[0];
    RTObject              model_square     = ImportObj::load_obj(MODEL_SQUARE)[0];

    Shader2          shader_depth = {EXAMPLE_CUR_PATH + "shader/depth.vert",
                                     EXAMPLE_CUR_PATH + "shader/depth.frag"};
    Shader2          shader_pcss  = {EXAMPLE_CUR_PATH + "shader/pcss.vert",
                                     EXAMPLE_CUR_PATH + "shader/pcss.frag"};
    ShaderDiffuse    shader_lambert;
    ShaderBlinnPhong shader_phong;
    ShaderTexVisual  shader_texvisual;

    int                   scene_switcher = 0;
    std::vector<RTObject> scene;

    struct {
        RTObject                model          = ImportObj::load_obj(MODEL_LIGHT)[0];
        glm::vec3               shadow_map_dir = {1, 2, 3};
        [[nodiscard]] glm::mat4 get_view() const
        {
            return glm::lookAt(model.position(), {0, 0, 0}, POSITIVE_Y);
        }
        glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 50.0f);
    } light;

    void init() override
    {
        glClearColor(0.6, 0.6, 0.6, 1.0);

        shader_lambert.init(camera.proj_matrix());
        light.model.set_pos({-5.8, 5.8, 3.5});
        shader_phong.init(camera.proj_matrix());
    }

    void tick_pre_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer.frame_buffer);
        glViewport(0, 0, buffer.size, buffer.size);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_depth.set_uniform({
                {"m_view", light.get_view()},
                {"m_proj", light.proj},
        });
        switch (scene_switcher)
        {
            case 0:
                scene = model_diona;
                scene.push_back(model_floor);
                break;
            case 1: scene = model_matrix; break;
            case 2: scene = model_three_obj; break;
            case 3:
                scene = model_202;
                scene.push_back(model_gray_floor);
                break;
            default: scene = {};
        }
        for (const auto &m: scene)
        {
            shader_depth.set_uniform({{"m_model", m.matrix()}});
            m.mesh.draw();
        }

        glBindTexture(GL_TEXTURE_2D, buffer.shadow_map);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void tick_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Window::framebuffer_width(), Window::framebuffer_width());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_pcss.set_uniform({
                {"m_view", camera.view_matrix()},
                {"m_proj", camera.proj_matrix()},
                {"light_vp", light.proj * light.get_view()},
                {"light_pos", light.model.position()},
                {"camera_pos", camera.get_pos()},
                {"rand_seed", glm::vec3(std::rand(), std::rand(), 0)},
        });

        glBindTexture_(GL_TEXTURE_2D, 0, buffer.shadow_map);
        for (const auto &m: scene)
        {
            const Material &mat = m.mesh.mat;
            if (mat.has_tex_basecolor())
                glBindTexture_(GL_TEXTURE_2D, 1, mat.metallic_roughness.tex_base_color);
            shader_pcss.set_uniform({
                    {"kd", glm::vec3(mat.metallic_roughness.base_color)},
                    {"ks", glm::vec3(0.3f)},
                    {"has_diffuse", mat.has_tex_basecolor()},
                    {"tex_diffuse", 1},
                    {"shadow_map", 0},
                    {"m_model", m.matrix()},
            });
            m.mesh.draw();
        }

        shader_lambert.update_per_fame(camera.view_matrix());
        shader_lambert.draw(light.model);

        // debug visual shadow map
        glViewport(0, 0, Window::framebuffer_width() / 4, Window::framebuffer_height() / 4);
        shader_texvisual.draw(model_square, buffer.shadow_map);
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y,
                    camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw,
                    camera.get_euler().pitch);

        {
            glm::vec3 pos = light.model.position();
            ImGui::SliderFloat("light pos x", &pos.x, -10.f, 10.f);
            ImGui::SliderFloat("light pos y", &pos.y, -10.f, 10.f);
            ImGui::SliderFloat("light pos z", &pos.z, -10.f, 10.f);
            light.model.set_pos(pos);
        }
        ImGui::SliderInt("scene switcher", &scene_switcher, 0, 3);
        ImGui::End();
    }
};


int main()
{
    auto engine = TestEngine();
    engine.engine_main();
}
