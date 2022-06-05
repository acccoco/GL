#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/misc.h"
#include "core/model.h"
#include "core/light.h"
#include "core/texture.h"


class ShaderRayMarch : public Shader
{
public:
    UniformAttribute light_pos    = {"light_pos", this, VEC3};
    UniformAttribute light_color  = {"light_color", this, VEC3};
    UniformAttribute camera_fov   = {"camera_fov", this, FLOAT};
    UniformAttribute camera_pos   = {"camera_pos", this, VEC3};
    UniformAttribute camera_front = {"camera_front", this, VEC3};
    UniformAttribute camera_up    = {"camera_up", this, VEC3};
    UniformAttribute camera_right = {"camera_right", this, VEC3};
    UniformAttribute near         = {"near", this, FLOAT};

    ShaderRayMarch()
        : Shader(EXAMPLE_CUR_PATH + "shader/ray-march.vert", EXAMPLE_CUR_PATH + "shader/ray-march.frag")
    {
        this->uniform_attrs_location_init();
    }
};


class RayMarch : public Engine
{
    const float near = 0.1f;    // 近平面到摄像机的距离
    const float fov  = 90.f;

    ShaderRayMarch shader_raymarch;

    PointLight point_light{.pos = {1.f, 2.f, 3.f}, .color = {0.9f, 0.9f, 0.9f}};

    Model canvas = Model::load_obj(MODEL_SQUARE)[0];

    void init() override
    {
        shader_raymarch.set_uniform({
                {shader_raymarch.near, {._float = near}},
                {shader_raymarch.camera_fov, {._float = fov}},

        });
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 camera_right = camera.get_right();
        glm::vec3 camera_front = camera.get_front();
        glm::vec3 camera_up    = glm::normalize(glm::cross(camera_right, camera_front));
        shader_raymarch.set_uniform({
                {shader_raymarch.camera_front, {._vec3 = camera_front}},
                {shader_raymarch.camera_right, {._vec3 = camera_right}},
                {shader_raymarch.camera_up, {._vec3 = camera_up}},
                {shader_raymarch.camera_pos, {._vec3 = camera.get_pos()}},
                {shader_raymarch.light_pos, {._vec3 = point_light.pos}},
                {shader_raymarch.light_color, {._vec3 = point_light.color}},
        });
        canvas.mesh.draw();
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");

        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);

        ImGui::SliderFloat("light pos x", &point_light.pos.x, -10.f, 10.f);
        ImGui::SliderFloat("light pos y", &point_light.pos.y, -10.f, 10.f);
        ImGui::SliderFloat("light pos z", &point_light.pos.z, -10.f, 10.f);
        ImGui::ColorPicker3("light color", &point_light.color[0]);

        ImGui::End();
    }
};


int main()
{
    auto a = RayMarch();
    a.engine_main();
}