/**
 * 生成 cube-map 形式的 shadow map，然后使用 shadow mapping 实现硬阴影。
 */

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/misc.h"
#include "core/model.h"
#include "core/texture.h"

#include "shader/diffuse/diffuse.h"


class ShaderDepth : public Shader
{
public:
    UniformAttribute m_view{"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj{"m_proj", this, UniAttrType::MAT4};
    UniformAttribute m_model{"m_model", this, UniAttrType::MAT4};

    ShaderDepth()
        : Shader(EXAMPLE_CUR_PATH + "shader/distance-to-light.vert", EXAMPLE_CUR_PATH + "shader/distance-to-light.frag")
    {
        this->uniform_attrs_location_init();
    }
};


class ShaderShadowMapping : public Shader
{
public:
    UniformAttribute m_view  = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj  = {"m_proj", this, UniAttrType::MAT4};
    UniformAttribute m_model = {"m_model", this, UniAttrType::MAT4};

    UniformAttribute light_pos  = {"light_pos", this, UniAttrType::VEC3};
    UniformAttribute camera_pos = {"camera_pos", this, UniAttrType::VEC3};

    UniformAttribute shadow_map_cube = {"shadow_map_cube", this, UniAttrType::INT};

    UniformAttribute has_diffuse = {"has_diffuse", this, UniAttrType::INT};
    UniformAttribute tex_diffuse = {"tex_diffuse", this, UniAttrType::INT};
    UniformAttribute kd          = {"kd", this, UniAttrType::VEC3};
    UniformAttribute ks          = {"ks", this, UniAttrType::VEC3};

    ShaderShadowMapping()
        : Shader(EXAMPLE_CUR_PATH + "shader/shadow-mapping.vert", EXAMPLE_CUR_PATH + "shader/shadow-mapping.frag")
    {
        this->uniform_attrs_location_init();
    }
};


class EngineTest : public Engine
{
private:
    GLuint frame_buffer{};
    GLuint depth_buffer{};
    GLuint cube_shadow_map{};

    const GLsizei frame_buffer_size = 1024;

    std::vector<Model> model_three_obj = Model::load_obj(MODEL_THREE_OBJS);
    std::vector<Model> model_matrix    = Model::load_obj(MODEL_SPHERE_MATRIX);
    std::vector<Model> model_202       = Model::load_obj(MODEL_202_CHAN);
    std::vector<Model> model_diona     = Model::load_obj(MODEL_DIONA);
    Model              model_light     = Model::load_obj(MODEL_LIGHT)[0];
    Model              model_floor     = Model::load_obj(MODLE_FLOOR)[0];

    ShaderDepth         shader_depth;
    ShaderShadowMapping shader_shadow;
    ShaderDiffuse       shader_diffuse;

    const int SCENE_MAX_CNT  = 4;    // 场景的总数
    int       scene_switcher = 0;    // 当前选中哪个场景

    /// 场景的详细信息
    std::vector<std::vector<Model>> scenes = std::vector<std::vector<Model>>(SCENE_MAX_CNT);

protected:
    void init() override
    {
        /// init frame buffer
        {
            glGenFramebuffers(1, &frame_buffer);
            glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

            depth_buffer = create_depth_buffer(frame_buffer_size, frame_buffer_size);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

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
        model_light.pos = {3, 4, 5};
    }


    void tick_render() override
    {
        std::vector<Model> &scene = scenes[scene_switcher];
        shadow_pass(scene);
        color_pass(scene);
    }


    void shadow_pass(const std::vector<Model> &scene)
    {
        // generate depth cube map
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        glViewport(0, 0, frame_buffer_size, frame_buffer_size);

        /// 重点：确保视角是 90 度
        glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 20.f);

        /// 将场景绘制到 cube map 的某个面上
        auto draw_dir = [&](GLenum textarget, const glm::vec3 &front, const glm::vec3 &up) {
            // textarget: for cube map, specify which face is to be attached
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textarget, cube_shadow_map, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 m_view = glm::lookAt(model_light.pos, model_light.pos + front, up);

            shader_depth.set_uniform({
                    {shader_depth.m_view, {._mat4 = m_view}},
                    {shader_depth.m_proj, {._mat4 = proj}},
            });

            for (auto &m: scene)
            {
                shader_depth.set_uniform({
                        {shader_depth.m_model, {._mat4 = m.model_matrix()}},
                });
                m.mesh.draw();
            }
        };

        /// 绘制某个面时，需要将摄像机的 up 和 front 调整为如下值
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_X, CameraDirDrawCube::pos_x.front, CameraDirDrawCube::pos_x.up);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, CameraDirDrawCube::neg_x.front, CameraDirDrawCube::neg_x.up);
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, CameraDirDrawCube::pos_z.front, CameraDirDrawCube::pos_z.up);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, CameraDirDrawCube::neg_z.front, CameraDirDrawCube::neg_z.up);
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, CameraDirDrawCube::pos_y.front, CameraDirDrawCube::pos_y.up);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, CameraDirDrawCube::neg_y.front, CameraDirDrawCube::neg_y.up);
    }


    void color_pass(const std::vector<Model> &scene)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport_(0, 0, window.width, window.height);

        // phong with shadow mapping
        glBindTexture_(GL_TEXTURE_CUBE_MAP, 0, cube_shadow_map);
        shader_shadow.set_uniform({
                {shader_shadow.m_view, {._mat4 = camera.view_matrix()}},
                {shader_shadow.m_proj, {._mat4 = Camera::proj_matrix()}},
                {shader_shadow.camera_pos, {._vec3 = camera.get_pos()}},
                {shader_shadow.light_pos, {._vec3 = model_light.pos}},
                {shader_shadow.shadow_map_cube, {._int = 0}},
        });

        for (auto &m: scene)
        {
            if (m.tex_diffuse.has)
                glBindTexture_(GL_TEXTURE_2D, 1, m.tex_diffuse.id);
            shader_shadow.set_uniform({
                    {shader_shadow.m_model, {._mat4 = m.model_matrix()}},
                    {shader_shadow.kd, {._vec3 = m.color_diffuse}},
                    {shader_shadow.ks, {._vec3 = m.color_specular}},
                    {shader_shadow.has_diffuse, {._int = m.tex_diffuse.has}},
                    {shader_shadow.tex_diffuse, {._int = 1}},
            });
            m.mesh.draw();
        }

        // light visualization
        shader_diffuse.set_uniform({
                {shader_diffuse.m_view, {._mat4 = camera.view_matrix()}},
                {shader_diffuse.m_proj, {._mat4 = Camera::proj_matrix()}},
        });
        shader_diffuse.draw(model_light);
    }


    void tick_gui() override
    {
        ImGui::Begin("setting");

        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);

        ImGui::SliderInt("scene switcher", &scene_switcher, 0, SCENE_MAX_CNT - 1);

        ImGui::SliderFloat("light x", &model_light.pos.x, -10, 10);
        ImGui::SliderFloat("light y", &model_light.pos.y, -10, 10);
        ImGui::SliderFloat("light z", &model_light.pos.z, -10, 10);
        ImGui::End();
    }
};

int main()
{
    auto engine = EngineTest();
    engine.engine_main();
}