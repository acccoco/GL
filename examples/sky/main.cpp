// holy shit about cube map's coordinate
// https://www.reddit.com/r/opengl/comments/cniubw/flipped_cube_maps/

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/model.h"
#include "function/skybox/skybox.h"

#include "shader/lambert/lambert.h"


class EngineTest : public Engine
{
    ShaderLambert shader_lambert;

    std::vector<Model> model_202 = Model::load_obj(MODEL_202_CHAN);

    SkyBox skybox;

    void init() override
    {
        glDepthFunc(GL_LEQUAL);
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);

        shader_lambert.init(Camera::proj_matrix());
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.draw_default_sky(camera.view_matrix(), Camera::proj_matrix());

        shader_lambert.update_per_fame(camera.view_matrix());
        for (auto &m: model_202)
            shader_lambert.draw(m);
    }

    void tick_gui() override
    {
        ImGui::Begin("camera");
        ImGui::Text("Eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);
        ImGui::End();
    }
};

int main()
{
    auto engine = EngineTest();
    engine.engine_main();
}