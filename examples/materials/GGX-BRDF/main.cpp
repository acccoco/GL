#include <array>
#include <string>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fmt/format.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/light.h"
#include "core/misc.h"


class AnisotropicBRDF : public Engine
{
    RTObject model_sphere = ImportObj::load_obj(MODEL_SPHERE)[0];
    RTObject model_floor  = ImportObj::load_obj(MODEL_GRAY_FLOOR)[0];
    RTObject model_cube   = ImportObj::load_obj(MODEL_CUBE)[0];

    GLuint tex_albedo =
            TextureManager::load_texture_(fmt::format("{}{}", TEXTURE_PBR_BALL, "basecolor.png"));
    GLuint tex_roughness =
            TextureManager::load_texture_(fmt::format("{}{}", TEXTURE_PBR_BALL, "roughness.png"));

    PointLight point_light{.pos = {0.f, 5.f, 3.f}, .color = {0.7f, 0.7f, 0.7f}};
    glm::vec3  ambient_color = {0.2f, 0.2f, 0.2f};

    Shader2 shader_brdf = {EXAMPLE_CUR_PATH + "shader/brdf.vert",
                           EXAMPLE_CUR_PATH + "shader/brdf.frag"};

    void init() override {}

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_brdf.set_uniform({
                {"m_proj", camera.proj_matrix()},
                {"m_view", camera.view_matrix()},
                {"camera_pos", camera.get_pos()},
                {"light_pos", point_light.pos},
                {"light_color", point_light.color},
                {"ambient_light", ambient_color},
        });

        glBindTexture_(GL_TEXTURE_2D, 0, tex_albedo);
        glBindTexture_(GL_TEXTURE_2D, 1, tex_roughness);
        shader_brdf.set_uniform({
                {"m_model", model_floor.matrix()},
                {"tex_albedo", 0},
                {"tex_roughness", 1},
        });
        model_floor.mesh.draw();
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y,
                    camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw,
                    camera.get_euler().pitch);
        ImGui::SliderFloat("light pos x", &point_light.pos.x, -10.f, 10.f);
        ImGui::SliderFloat("light pos y", &point_light.pos.y, -10.f, 10.f);
        ImGui::SliderFloat("light pos z", &point_light.pos.z, -10.f, 10.f);
        ImGui::ColorPicker3("ambient color", &ambient_color[0]);
        ImGui::ColorPicker3("light color", &point_light.color[0]);
        ImGui::End();
    }
};


int main()
{
    auto a = AnisotropicBRDF();
    a.engine_main();
}