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
#include "core/model.h"
#include "core/texture.h"

#include "shader/tex2d-visual/tex-visual.h"
#include "shader/diffuse/diffuse.h"
#include "shader/blinn-phong/blinn-phong.h"

#include "./shader.h"


struct DepthFramebuffer {
    GLuint frame_buffer{};
    GLuint depth_buffer;
    GLuint shadow_map;
    const GLsizei size = 1024;

    DepthFramebuffer()
    {
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        depth_buffer = create_depth_buffer(size, size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

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

    std::vector<Model> model_three_obj = Model::load_obj(MODEL_THREE_OBJS);
    std::vector<Model> model_matrix = Model::load_obj(MODEL_SPHERE_MATRIX);
    std::vector<Model> model_202 = Model::load_obj(MODEL_202_CHAN);
    std::vector<Model> model_diona = Model::load_obj(MODEL_DIONA);
    Model model_cube = Model::load_obj(MODEL_CUBE)[0];
    Model model_light = Model::load_obj(MODEL_LIGHT)[0];
    Model model_floor = Model::load_obj(MODLE_FLOOR)[0];
    Model model_gray_floor = Model::load_obj(MODEL_GRAY_FLOOR)[0];
    Model model_square = Model::load_obj(MODEL_SQUARE)[0];

    ShaderDepth shader_depth;
    ShaderPcss shader_pcss;
    ShaderDiffuse shader_lambert;
    ShaderBlinnPhong shader_phong;
    ShaderTexVisual shader_texvisual;

    int scene_switcher = 0;

    struct {
        Model model = Model::load_obj(MODEL_LIGHT)[0];
        glm::vec3 shadow_map_dir = {1, 2, 3};
        [[nodiscard]] glm::mat4 get_view() const
        {
            return glm::lookAt(model.pos, {0, 0, 0}, POSITIVE_Y);
        }
        glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 50.0f);
    } light;

    void init() override
    {
        glClearColor(0.6, 0.6, 0.6, 1.0);

        shader_lambert.init(Camera::proj_matrix());
        light.model.pos = {-5.8, 5.8, 3.5};
        shader_phong.init(Camera::proj_matrix());
    }

    void tick_pre_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer.frame_buffer);
        glViewport(0, 0, buffer.size, buffer.size);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_depth.update_per_frame(light.get_view(), light.proj);
        switch (scene_switcher) {
            case 0:
                for (auto &m: model_diona)
                    shader_depth.draw(m);
                shader_depth.draw(model_floor);
                break;
            case 1:
                for (auto &m: model_matrix)
                    shader_depth.draw(m);
                break;
            case 2:
                for (auto &m: model_three_obj)
                    shader_depth.draw(m);
                break;
            case 3:
                for (auto &m: model_202)
                    shader_depth.draw(m);
                shader_depth.draw(model_gray_floor);
            default:;
        }

        glBindTexture(GL_TEXTURE_2D, buffer.shadow_map);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void tick_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport_(0, 0, window.width, window.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_pcss.udpate_per_frame(camera.view_matrix(), Camera::proj_matrix(), light.proj * light.get_view(),
                                     light.model.pos, camera.get_pos());

        switch (scene_switcher) {
            case 0:
                for (auto &m: model_diona)
                    shader_pcss.draw(m, buffer.shadow_map);
                shader_pcss.draw(model_floor, buffer.shadow_map);
                break;
            case 1:
                for (auto &m: model_matrix)
                    shader_pcss.draw(m, buffer.shadow_map);
                break;
            case 2:
                for (auto &m: model_three_obj)
                    shader_pcss.draw(m, buffer.shadow_map);
                break;
            case 3:
                for (auto &m: model_202)
                    shader_pcss.draw(m, buffer.shadow_map);
                shader_pcss.draw(model_gray_floor, buffer.shadow_map);
            default:;
        }

        shader_lambert.update_per_fame(camera.view_matrix());
        shader_lambert.draw(light.model);

        // debug visual shadow map
        glViewport_(0, 0, window.width / 4, window.height / 4);
        shader_texvisual.draw(model_square, buffer.shadow_map);
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);
        ImGui::SliderFloat("light pos x", &light.model.pos.x, -10.f, 10.f);
        ImGui::SliderFloat("light pos y", &light.model.pos.y, -10.f, 10.f);
        ImGui::SliderFloat("light pos z", &light.model.pos.z, -10.f, 10.f);
        ImGui::SliderInt("scene switcher", &scene_switcher, 0, 3);
        ImGui::End();
    }
};


int main()
{
    auto engine = TestEngine();
    engine.engine_main();
}
