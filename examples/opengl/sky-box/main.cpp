/**
 * 实现天空盒
 * holy shit about cube map's coordinate
 * https://www.reddit.com/r/opengl/comments/cniubw/flipped_cube_maps/
 */

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"

#include "shader/skybox/skybox.h"
#include "shader/diffuse/diffuse.h"


class EngineTest : public Engine
{
    ShaderDiffuse shader_lambert;

    std::vector<RTObject> model_202 = ImportObj::load_obj(MODEL_202_CHAN);

    SkyBox        skybox;
    CubeMapVisual visual;

    GLuint cube_cornell = load_cube_map({
            .pos_x = CUBE_CORNELL + "posx.jpg",
            .neg_x = CUBE_CORNELL + "negx.jpg",
            .pos_y = CUBE_CORNELL + "posy.jpg",
            .neg_y = CUBE_CORNELL + "negy.jpg",
            .pos_z = CUBE_CORNELL + "posz.jpg",
            .neg_z = CUBE_CORNELL + "negz.jpg",
    });

    GLuint cube_sky2 = load_cube_map({
            .pos_x = CUBE_SKY2 + "posx.jpg",
            .neg_x = CUBE_SKY2 + "negx.jpg",
            .pos_y = CUBE_SKY2 + "posy.jpg",
            .neg_y = CUBE_SKY2 + "negy.jpg",
            .pos_z = CUBE_SKY2 + "posz.jpg",
            .neg_z = CUBE_SKY2 + "negz.jpg",
    });

    void init() override
    {
        glDepthFunc(GL_LEQUAL);
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);

        shader_lambert.init(camera.proj_matrix());
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // skybox.draw_default_sky(camera.view_matrix(), Camera::proj_matrix());
        visual.draw_as_skybox(camera.view_matrix(), camera.proj_matrix(), cube_sky2);

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