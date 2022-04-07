#include <iostream>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/model.h"


#include "shader/lambert/lambert.h"
#include "shader/blinn-phong/blinn-phong.h"


class SceneDiffuse : public Engine
{
private:
    // model
    std::vector<Model> model_202 = Model::load_obj(MODEL_202_CHAN);
    std::vector<Model> model_matrix = Model::load_obj(MODEL_SPHERE_MATRIX);
    Model model_cube = Model::load_obj(MODEL_CUBE)[0];
    Model model_square = Model::load_obj(MODEL_SQUARE)[0];
    Model model_light = Model::load_obj(MODEL_LIGHT)[0];

    // shader
    ShaderLambert shader_lambert;
    ShaderBlinnPhong shader_phong;

    // scene
    int scene_switcher = 0;

    void init() override
    {
        shader_lambert.init(Camera::proj_matrix());
        shader_phong.init(Camera::proj_matrix());

        model_cube.pos = glm::vec3{2, 1, 0};
        model_square.pos = glm::vec3{-2, 1, 0};
        model_light.pos = glm::vec3(-3, 4, 4);
    }

    void tick_render() override
    {
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        switch (scene_switcher) {
            case 0: scene_0(); break;
            case 1: scene_1(); break;
            case 2: scene_2(); break;
            case 3: scene_3(); break;
            default: scene_0();
        }
    }

    void scene_0()
    {
        // lambert
        shader_lambert.update_per_fame(camera.view_matrix());
        shader_lambert.draw(model_cube);
        shader_lambert.draw(model_square);
    }

    void scene_1()
    {
        // phong
        shader_lambert.update_per_fame(camera.view_matrix());
        shader_lambert.draw(model_light);

        shader_phong.update_per_frame(camera.view_matrix(), camera.get_pos(), model_light.pos, 2.0);
        for (auto &m: model_matrix)
            shader_phong.draw(m);
    }

    void scene_2()
    {
        // phong, Marry
        shader_phong.update_per_frame(camera.view_matrix(), camera.get_pos(), model_light.pos, 2.0);
        for (auto &m: model_202)
            shader_phong.draw(m);

        shader_lambert.update_per_fame(camera.view_matrix());
        shader_lambert.draw(model_light);
    }

    void scene_3()
    {
        shader_lambert.update_per_fame(camera.view_matrix());
        for (auto &m: model_matrix)
            shader_lambert.draw(m);
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::SliderInt("scene switcher", &scene_switcher, 0, 3);
        ImGui::SliderFloat("light x", &model_light.pos.x, -10, 10);
        ImGui::SliderFloat("light y", &model_light.pos.y, -10, 10);
        ImGui::SliderFloat("light z", &model_light.pos.z, -10, 10);
        ImGui::End();
    }
};

int main()
{
    auto engine = SceneDiffuse();
    engine.engine_main();
}
