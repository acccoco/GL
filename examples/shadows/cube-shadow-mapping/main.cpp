/**
 * 生成 cube-map 形式的 shadow map，然后使用 shadow mapping 实现硬阴影。
 */

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/misc.h"
#include "core/texture.h"

#include "shader/diffuse/diffuse.h"


class EngineTest : public Engine
{
private:
    GLuint frame_buffer{};
    GLuint depth_buffer{};
    GLuint cube_shadow_map{};

    const GLsizei frame_buffer_size = 1024;

    std::vector<RTObject> model_three_obj = ImportObj::load_obj(MODEL_THREE_OBJS);
    std::vector<RTObject> model_matrix    = ImportObj::load_obj(MODEL_SPHERE_MATRIX);
    std::vector<RTObject> model_202       = ImportObj::load_obj(MODEL_202_CHAN);
    std::vector<RTObject> model_diona     = ImportObj::load_obj(MODEL_DIONA);
    RTObject              model_light     = ImportObj::load_obj(MODEL_LIGHT)[0];
    RTObject              model_floor     = ImportObj::load_obj(MODLE_FLOOR)[0];

    Shader2       shader_depth  = {EXAMPLE_CUR_PATH + "shader/distance-to-light.vert",
                                   EXAMPLE_CUR_PATH + "shader/distance-to-light.frag"};
    Shader2       shader_shadow = {EXAMPLE_CUR_PATH + "shader/shadow-mapping.vert",
                                   EXAMPLE_CUR_PATH + "shader/shadow-mapping.frag"};
    ShaderDiffuse shader_diffuse;

    const int SCENE_MAX_CNT  = 4;    // 场景的总数
    int       scene_switcher = 0;    // 当前选中哪个场景

    /// 场景的详细信息
    std::vector<std::vector<RTObject>> scenes = std::vector<std::vector<RTObject>>(SCENE_MAX_CNT);

protected:
    void init() override
    {
        shader_diffuse.init(camera.proj_matrix());

        /// init frame buffer
        {
            glGenFramebuffers(1, &frame_buffer);
            glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

            depth_buffer = create_depth_buffer(frame_buffer_size, frame_buffer_size);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                      depth_buffer);

            cube_shadow_map = new_cubemap({
                    .size            = frame_buffer_size,
                    .internal_format = GL_RGB32F,
                    .external_format = GL_RGB,
                    .external_type   = GL_FLOAT,
            });
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        /// 场景信息
        {
            combine(scenes[0], model_202);
            scenes[0].push_back(model_floor);

            combine(scenes[1], model_three_obj);

            combine(scenes[2], model_matrix);

            combine(scenes[3], model_diona);
            scenes[3].push_back(model_floor);
        }

        glDepthFunc(GL_LEQUAL);
        model_light.set_pos({3, 4, 5});
    }


    void tick_render() override
    {
        std::vector<RTObject> &scene = scenes[scene_switcher];
        shadow_pass(scene);
        color_pass(scene);
    }


    void shadow_pass(const std::vector<RTObject> &scene)
    {
        // generate depth cube map
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        glViewport(0, 0, frame_buffer_size, frame_buffer_size);

        /// 重点：确保视角是 90 度
        glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 20.f);

        /// 将场景绘制到 cube map 的某个面上
        auto draw_dir = [&](GLenum textarget, const glm::vec3 &front, const glm::vec3 &up) {
            // textarget: for cube map, specify which face is to be attached
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textarget, cube_shadow_map,
                                   0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 m_view =
                    glm::lookAt(model_light.position(), model_light.position() + front, up);

            shader_depth.set_uniform({
                    {"m_view", m_view},
                    {"m_proj", proj},
            });

            for (auto &m: scene)
            {
                shader_depth.set_uniform({
                        {"m_model", m.matrix()},
                });
                m.mesh.draw();
            }
        };

        /// 绘制某个面时，需要将摄像机的 up 和 front 调整为如下值
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_X, CameraDirDrawCube::pos_x.front,
                 CameraDirDrawCube::pos_x.up);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, CameraDirDrawCube::neg_x.front,
                 CameraDirDrawCube::neg_x.up);
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, CameraDirDrawCube::pos_z.front,
                 CameraDirDrawCube::pos_z.up);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, CameraDirDrawCube::neg_z.front,
                 CameraDirDrawCube::neg_z.up);
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, CameraDirDrawCube::pos_y.front,
                 CameraDirDrawCube::pos_y.up);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, CameraDirDrawCube::neg_y.front,
                 CameraDirDrawCube::neg_y.up);
    }


    void color_pass(const std::vector<RTObject> &scene)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, Window::framebuffer_width(), Window::framebuffer_height());

        // phong with shadow mapping
        glBindTexture_(GL_TEXTURE_CUBE_MAP, 0, cube_shadow_map);
        shader_shadow.set_uniform({
                {"m_view", camera.view_matrix()},
                {"m_proj", camera.proj_matrix()},
                {"camera_pos", camera.get_pos()},
                {"light_pos", model_light.position()},
                {"shadow_map_cube", 0},
        });

        for (auto &m: scene)
        {
            if (m.mesh.mat.has_tex_basecolor())
                glBindTexture_(GL_TEXTURE_2D, 1, m.mesh.mat.metallic_roughness.tex_base_color);
            shader_shadow.set_uniform({
                    {"m_model", m.matrix()},
                    {"kd", glm::vec3(m.mesh.mat.metallic_roughness.base_color)},
                    {"ks", glm::vec3(0.5f)},
                    {"has_diffuse", m.mesh.mat.has_tex_basecolor()},
                    {"tex_diffuse", 1},
            });
            m.mesh.draw();
        }

        // light visualization
        shader_diffuse.update_per_fame(camera.view_matrix());
        shader_diffuse.draw(model_light);
    }


    void tick_gui() override
    {
        ImGui::Begin("setting");

        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y,
                    camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw,
                    camera.get_euler().pitch);

        ImGui::SliderInt("scene switcher", &scene_switcher, 0, SCENE_MAX_CNT - 1);

        {
            glm::vec3 light_pos = model_light.position();
            ImGui::SliderFloat("light x", &light_pos.x, -10, 10);
            ImGui::SliderFloat("light y", &light_pos.y, -10, 10);
            ImGui::SliderFloat("light z", &light_pos.z, -10, 10);
            model_light.set_pos(light_pos);
        }

        ImGui::End();
    }
};

int main()
{
    auto engine = EngineTest();
    engine.engine_main();
}