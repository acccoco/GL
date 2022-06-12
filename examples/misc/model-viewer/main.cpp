/**
 * usage:
 * model-viewer [model1.obj] [model2.obj] ...
 * 使用 blinn-phong shader 渲染给定模型
 */

#include <iostream>
#include <iostream>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"


#include "shader/diffuse/diffuse.h"
#include "shader/blinn-phong/blinn-phong.h"

class TestEngine : public Engine
{
    // shader
    ShaderBlinnPhong shader_phong;

    // model
    std::vector<RTObject> models;

    // light
    glm::vec3 light_pos{-3, 4, 4};

    void init() override
    {
        // load model
        for (auto &path: model_path_list)
        {
            auto model = ImportObj::load_obj(path);
            models.insert(models.end(), model.begin(), model.end());
        }

        // init shader
        shader_phong.init(camera.proj_matrix());

        // init others
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
    }
    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_phong.update_per_frame(camera.view_matrix(), camera.get_pos(), light_pos, 2.0);
        for (auto &m: models)
        {
            shader_phong.draw(m);
        }
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y,
                    camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw,
                    camera.get_euler().pitch);
        ImGui::SliderFloat("light pos x", &light_pos.x, -10.f, 10.f);
        ImGui::SliderFloat("light pos y", &light_pos.y, -10.f, 10.f);
        ImGui::SliderFloat("light pos z", &light_pos.z, -10.f, 10.f);
        ImGui::End();
    }

public:
    std::vector<std::string> model_path_list;
};


int main(int args, char **argv)
{
    if (args == 1)
    {
        std::cout << "cmd [model1.obj] [model2.obj]" << std::endl;
        return 0;
    }

    auto engine = TestEngine();
    for (int i = 1; i < args; ++i)
    {
        engine.model_path_list.emplace_back(argv[i]);
    }

    engine.engine_main();
}