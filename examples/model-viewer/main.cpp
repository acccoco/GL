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
#include "core/model.h"


#include "shader/lambert/lambert.h"
#include "shader/blinn-phong/blinn-phong.h"

class TestEngine : public Engine
{
    // shader
    ShaderBlinnPhong shader_phong;

    // model
    std::vector<Model> models;

    // light
    glm::vec3 light_pos{-3, 4, 4};

    void init() override
    {
        // load model
        for (auto &path: model_path_list) {
            auto model = Model::load_obj(path);
            models.insert(models.end(), model.begin(), model.end());
        }

        // init shader
        shader_phong.init(Camera::proj_matrix());

        // init others
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
    }
    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_phong.update_per_frame(camera.view_matrix(), camera.get_pos(), light_pos, 2.0);
        for (auto &m: models) {
            shader_phong.draw(m);
        }
    }

public:
    std::vector<std::string> model_path_list;
};


int main(int args, char **argv)
{
    if (args == 1) {
        std::cout << "cmd [model1.obj] [model2.obj]" << std::endl;
        return 0;
    }

    auto engine = TestEngine();
    for (int i = 1; i < args; ++i) {
        engine.model_path_list.emplace_back(argv[i]);
    }

    engine.engine_main();
}