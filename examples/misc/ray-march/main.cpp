#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/misc.h"
#include "core/light.h"
#include "core/texture.h"


class RayMarch : public Engine
{
    const float near = 0.1f;    // 近平面到摄像机的距离
    const float fov  = 90.f;

    Shader2 shader_raymarch = {EXAMPLE_CUR_PATH + "shader/ray-march.vert",
                               EXAMPLE_CUR_PATH + "shader/ray-march.frag"};

    PointLight point_light{.pos = {1.f, 2.f, 3.f}, .color = {0.9f, 0.9f, 0.9f}};

    RTObject canvas = ImportObj::load_obj(MODEL_SQUARE)[0];

    void init() override
    {
        shader_raymarch.set_uniform({
                {"near", near},
                {"camera_fov", fov},

        });
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 camera_right = camera.get_right();
        glm::vec3 camera_front = camera.get_front();
        glm::vec3 camera_up    = glm::normalize(glm::cross(camera_right, camera_front));
        shader_raymarch.set_uniform({
                {"camera_front", camera_front},
                {"camera_right", camera_right},
                {"camera_up", camera_up},
                {"camera_pos", camera.get_pos()},
                {"light_pos", point_light.pos},
                {"light_color", point_light.color},
        });
        canvas.mesh.draw();
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
        ImGui::ColorPicker3("light color", &point_light.color[0]);

        ImGui::End();
    }
};


int main()
{
    auto a = RayMarch();
    a.engine_main();
}