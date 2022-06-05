#include <array>
#include <string>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fmt/format.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/model.h"
#include "core/light.h"
#include "core/misc.h"


class ShaderBRDF : public Shader
{
public:
    UniformAttribute m_view        = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj        = {"m_proj", this, UniAttrType::MAT4};
    UniformAttribute camera_pos    = {"camera_pos", this, UniAttrType::VEC3};
    UniformAttribute light_pos     = {"light_pos", this, UniAttrType::VEC3};
    UniformAttribute light_color   = {"light_color", this, UniAttrType::VEC3};
    UniformAttribute ambient_light = {"ambient_light", this, UniAttrType::VEC3};

    UniformAttribute m_model       = {"m_model", this, UniAttrType::MAT4};
    UniformAttribute tex_roughness = {"tex_roughness", this, UniAttrType::INT};
    UniformAttribute tex_albedo    = {"tex_albedo", this, UniAttrType::INT};

    ShaderBRDF() : Shader(EXAMPLE_CUR_PATH + "shader/brdf.vert", EXAMPLE_CUR_PATH + "shader/brdf.frag")
    {
        this->uniform_attrs_location_init();
    }
};


class AnisotropicBRDF : public Engine
{
    Model model_sphere = Model::load_obj(MODEL_SPHERE)[0];
    Model model_floor  = Model::load_obj(MODEL_GRAY_FLOOR)[0];
    Model model_cube   = Model::load_obj(MODEL_CUBE)[0];

    GLuint tex_albedo    = TextureManager::load_texture_(fmt::format("{}{}", TEXTURE_PBR_BALL, "basecolor.png"));
    GLuint tex_roughness = TextureManager::load_texture_(fmt::format("{}{}", TEXTURE_PBR_BALL, "roughness.png"));

    PointLight point_light{.pos = {0.f, 5.f, 3.f}, .color = {0.7f, 0.7f, 0.7f}};
    glm::vec3  ambient_color = {0.2f, 0.2f, 0.2f};

    ShaderBRDF shader_brdf;

    void init() override {}

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_brdf.set_uniform({
                {shader_brdf.m_proj, {._mat4 = Camera::proj_matrix()}},
                {shader_brdf.m_view, {._mat4 = camera.view_matrix()}},
                {shader_brdf.camera_pos, {._vec3 = camera.get_pos()}},
                {shader_brdf.light_pos, {._vec3 = point_light.pos}},
                {shader_brdf.light_color, {._vec3 = point_light.color}},
                {shader_brdf.ambient_light, {._vec3 = ambient_color}},
        });

        glBindTexture_(GL_TEXTURE_2D, 0, tex_albedo);
        glBindTexture_(GL_TEXTURE_2D, 1, tex_roughness);
        shader_brdf.set_uniform({
                {shader_brdf.m_model, {._mat4 = model_floor.model_matrix()}},
                {shader_brdf.tex_albedo, {._int = 0}},
                {shader_brdf.tex_roughness, {._int = 1}},
        });
        model_floor.mesh.draw();
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);
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