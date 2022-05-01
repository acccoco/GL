/**
 * 生成 cube-map 形式的 shadow map，然后使用 shadow mapping 实现硬阴影。
 */

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/misc.h"
#include "core/model.h"
#include "core/texture.h"

#include "shader/skybox/skybox.h"
#include "shader/lambert/lambert.h"

#include "./shader.h"


// shadow map framebuffer with cube color texture
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
private:
    CubeFramebuffer buffer;

    std::vector<Model> model_three_obj = Model::load_obj(MODEL_THREE_OBJS);
    std::vector<Model> model_matrix = Model::load_obj(MODEL_SPHERE_MATRIX);
    std::vector<Model> model_202 = Model::load_obj(MODEL_202_CHAN);
    std::vector<Model> model_diona = Model::load_obj(MODEL_DIONA);
    Model model_light = Model::load_obj(MODEL_LIGHT)[0];
    Model model_floor = Model::load_obj(MODLE_FLOOR)[0];

    ShaderDepth shader_depth;
    ShaderShadowMapping shader_shadow;
    ShaderLambert shader_lambert;

    SkyBox sky_box;

    const float light_near = 0.1;
    const float light_far = 20;

    int scene_switcher = 0;

protected:
    void init() override
    {
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
        glDepthFunc(GL_LEQUAL);

        shader_shadow.init(Camera::proj_matrix());
        shader_lambert.init(Camera::proj_matrix());

        model_light.pos = {3, 4, 5};
    }

    void tick_pre_render() override
    {
        // generate depth cube map
        glBindFramebuffer(GL_FRAMEBUFFER, buffer.frame_buffer);
        glViewport(0, 0, buffer.size, buffer.size);

        glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 20.f);

        auto draw_dir = [&](GLenum textarget, const glm::vec3 &front, const glm::vec3 &up) {
            // textarget: for cube map, specify which face is to be attached
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textarget, buffer.filtered_env_map, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 m_view = glm::lookAt(model_light.pos, model_light.pos + front, up);

            shader_depth.update_per_frame(m_view, proj);
            switch (scene_switcher) {
                case 0:
                    for (auto &m: model_202)
                        shader_depth.draw(m);
                    shader_depth.draw(model_floor);
                    break;
                case 1:
                    for (auto &m: model_three_obj)
                        shader_depth.draw(m);
                    break;
                case 2:
                    for (auto &m: model_matrix)
                        shader_depth.draw(m);
                    break;
                case 3:
                    for (auto &m: model_diona)
                        shader_depth.draw(m);
                    shader_depth.draw(model_floor);
                    break;
                default:;
            }
        };

        // for historical reason of cubemap, reverse UP direction of camera.
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_X, POSITIVE_X, -POSITIVE_Y);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, NEGATIVE_X, -POSITIVE_Y);
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, POSITIVE_Z, -POSITIVE_Y);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, NEGATIVE_Z, -POSITIVE_Y);// front
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, POSITIVE_Y, POSITIVE_Z);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, NEGATIVE_Y, NEGATIVE_Z);// down
    }

    void tick_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport_(0, 0, window.width, window.height);

        // phong with shadow mapping
        shader_shadow.update_per_frame(camera.view_matrix(), camera.get_pos(), model_light.pos);

        switch (scene_switcher) {
            case 0:
                for (auto &m: model_202)
                    shader_shadow.draw(m, buffer.filtered_env_map);
                shader_shadow.draw(model_floor, buffer.filtered_env_map);
                break;
            case 1:
                for (auto &m: model_three_obj)
                    shader_shadow.draw(m, buffer.filtered_env_map);
                break;
            case 2:
                for (auto &m: model_matrix)
                    shader_shadow.draw(m, buffer.filtered_env_map);
                break;
            case 3:
                for (auto &m: model_diona)
                    shader_shadow.draw(m, buffer.filtered_env_map);
                shader_shadow.draw(model_floor, buffer.filtered_env_map);
                break;
            default:;
        }
        // light visualization
        shader_lambert.update_per_fame(camera.view_matrix());
        shader_lambert.draw(model_light);
    }


    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);
        ImGui::SliderInt("scene switcher", &scene_switcher, 0, 3);
        ImGui::SliderFloat("light x", &model_light.pos.x, -10, 10);
        ImGui::SliderFloat("light y", &model_light.pos.y, -10, 10);
        ImGui::SliderFloat("light z", &model_light.pos.z, -10, 10);
        ImGui::End();
    }
};

int main()
{
    auto engine = EngineTest();
    engine.engine_main();
}