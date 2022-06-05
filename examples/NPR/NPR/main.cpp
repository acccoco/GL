/**
 * 目前还什么都没有做，只是进行了描边
 */

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/misc.h"
#include "core/model.h"
#include "core/shader2.h"


const std::string CUR_SHADER = EXAMPLE_CUR_PATH + "shader/";


class TestEngine : public Engine
{
    Shader2            shader{CUR_SHADER + "npr.vert", CUR_SHADER + "npr.frag"};
    glm::vec3          light_pos{-3, 4, 4};
    std::vector<Model> model_diona = Model::load_obj(MODEL_DIONA);
    std::vector<Model> scene;
    float              outline_threshold = 0.2f;

    void init() override
    {
        scene = model_diona;
        glClearColor(0.8f, 0.8f, 0.8f, 1.f);
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.set_uniform({
                {"m_view", camera.view_matrix()},
                {"m_proj", Camera::proj_matrix()},
                {"camera_pos", camera.get_pos()},
                {"light_pos", light_pos},
                {"light_indensity", 2.f},
                {"outline_threshold", outline_threshold},
        });

        for (auto &m: scene)
        {
            if (m.tex_diffuse.has)
                glBindTexture_(GL_TEXTURE_2D, 0, m.tex_diffuse.id);

            shader.set_uniform({
                    {"m_model", m.model_matrix()},
                    {"kd", m.color_diffuse},
                    {"ks", m.color_specular},
                    {"has_diffuse", m.tex_diffuse.has},
                    {"tex_diffuse", 0},
            });
            m.mesh.draw();
        }
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");

        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);

        ImGui::SliderFloat("light pos x", &light_pos.x, -10.f, 10.f);
        ImGui::SliderFloat("light pos y", &light_pos.y, -10.f, 10.f);
        ImGui::SliderFloat("light pos z", &light_pos.z, -10.f, 10.f);

        ImGui::SliderFloat("outline threshold", &outline_threshold, 0.f, 1.f);

        ImGui::End();
    }
};


int main(int args, char **argv)
{
    try
    {
        auto engine = TestEngine();
        engine.engine_main();
    } catch (std::exception &e)
    {
        SPDLOG_ERROR("exception.");
        exit(0);
    }
}