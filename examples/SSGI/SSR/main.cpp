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
#include "core/shader2.h"
#include "shader/tex2d-visual/tex-visual.h"
#include "shader/diffuse/diffuse.h"
#include "functions/axis.h"

const std::string CUR_SHADER = EXAMPLE_CUR_PATH + "shader/";


class SSR : public Engine
{
    /// 窗口的尺寸：1600 x 1600 用于显示，600 x 1600 用于展示 texture
    const int window_width  = 2400;
    const int window_height = 1600;

    /// shadow pass 需要的数据
    struct LightPassData {
        const GLsizei size = 512;

        GLuint framebuffer{};
        GLuint shadow_map   = new_tex2d({.width = size, .height = size, .internal_format = GL_R32F});
        GLuint depth_buffer = create_depth_buffer(size, size);

        Shader2 shader = Shader2(CUR_SHADER + "light-pass.vert", CUR_SHADER + "light-pass.frag");

        LightPassData()
        {
            glGenFramebuffers(1, &framebuffer);
            framebuffer_bind(framebuffer, depth_buffer, {shadow_map});
        }
    } light_pass_cfg;

    /// geometry pass 需要的数据
    struct GeometryPassData {
        const GLsizei size = 1600;

        GLuint framebuffer{};
        GLuint depth_buffer = create_depth_buffer(size, size);

        /// view-space 下的坐标
        GLuint tex_pos_view = new_tex2d({.width = size, .height = size, .internal_format = GL_RGBA32F});
        /// view-space 下的法线
        GLuint tex_normal_view = new_tex2d({.width = size, .height = size, .internal_format = GL_RGBA32F});
        /// 材质信息：diffuse albedo
        GLuint tex_diffuse = new_tex2d({.width = size, .height = size, .internal_format = GL_RGBA32F});

        Shader2 shader = Shader2(CUR_SHADER + "geometry-pass.vert", CUR_SHADER + "geometry-pass.frag");

        GeometryPassData()
        {
            glGenFramebuffers(1, &framebuffer);
            framebuffer_bind(framebuffer, depth_buffer, {tex_pos_view, tex_normal_view, tex_diffuse});
        }
    } geometry_pass_data;

    struct SSRPassData {
        const GLsizei size = 1600;

        GLuint framebuffer{};
        GLuint depth_buffer = create_depth_buffer(size, size);
        GLuint tex_ssr_uv   = new_tex2d({.width = size, .height = size, .internal_format = GL_RGBA32F});

        Shader2 shader = Shader2(CUR_SHADER + "ssr-pass.vert", CUR_SHADER + "ssr-pass.frag");

        SSRPassData()
        {
            glGenFramebuffers(1, &framebuffer);
            framebuffer_bind(framebuffer, depth_buffer, {tex_ssr_uv});
        }

    } ssr_pass_data;

    /// 场景中的方向光
    struct {
        glm::vec3 pos    = {2.f, 7.f, 7.f};
        glm::vec3 target = {-1.f, 0.f, 0.f};
        glm::vec3 color  = {0.7f, 0.7f, 0.7f};

        /// 这里是正交投影！！！！
        const glm::mat4         proj_matrix = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1e-2f, 100.f);
        [[nodiscard]] glm::mat4 view_matrix_get() const { return glm::lookAt(pos, target, POSITIVE_Y); }
        [[nodiscard]] glm::vec3 light_dir() const { return target - pos; }
    } light;

    /// 场景中的模型信息
    std::vector<Model> scene;
    std::vector<Model> model_three   = Model::load_obj(MODEL_THREE_OBJS);
    std::vector<Model> model_cornell = Model::load_obj(MODEL_CORNELL_BOX);

    Model model_square = Model::load_obj(MODEL_SQUARE)[0];
    Model model_cube   = Model::load_obj(MODEL_CUBE)[0];

    Shader2 shader = Shader2(CUR_SHADER + "color-pass.vert", CUR_SHADER + "color-pass.frag");

    /// 光源可视化
    ShaderDiffuse shader_diffuse;

    /// 纹理可视化
    ShaderTexVisual shader_tex_visual;

    /// 坐标轴
    Axis axis;


public:
    void init() override
    {
        this->set_window_size(window_width, window_height);

        scene = model_three;
    }

    void tick_render() override
    {
        glClearColor(0.f, 0.f, 0.f, 0.f);

        light_pass();
        geometry_pass();
        ssr_pass();
        color_pass();
        debug_pass();
    }

    void light_pass();

    void geometry_pass();

    void ssr_pass();

    void color_pass();

    void debug_pass();

    void tick_gui() override
    {
        ImGui::Begin("setting");

        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);

        ImGui::SliderFloat("light pos x", &light.pos.x, -10, 10);
        ImGui::SliderFloat("light pos y", &light.pos.y, -10, 10);
        ImGui::SliderFloat("light pos z", &light.pos.z, -10, 10);

        ImGui::SliderFloat("light target x", &light.target.x, -10, 10);
        ImGui::SliderFloat("light target y", &light.target.y, -10, 10);
        ImGui::SliderFloat("light target z", &light.target.z, -10, 10);

        ImGui::End();
    }
};


int main()
{
    try
    {
        auto a = SSR();
        a.engine_main();
    } catch (std::exception &e)
    {
        SPDLOG_ERROR("exception.");
        exit(0);
    }
}


/// ==================================================================


void SSR::debug_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ViewPortInfo viewport_info = {
            .width  = window.width,
            .height = window.height,
            .x_cnt  = 6,
            .y_cnt  = 4,
            .x_len  = 1,
            .y_len  = 1,
    };

    auto draw = [&](int x_idx, int y_idx, GLuint tex, int channel) {
        viewport_info.x_idx = x_idx;
        viewport_info.y_idx = y_idx;
        glViewport_(viewport_info);
        shader_tex_visual.draw(model_square, tex, channel);
    };

    /// light pass
    draw(0, 0, light_pass_cfg.shadow_map, 1);

    /// geometry pass
    draw(0, 1, geometry_pass_data.tex_pos_view, 0);
    draw(1, 1, geometry_pass_data.tex_normal_view, 0);
    draw(0, 2, geometry_pass_data.tex_diffuse, 0);

    /// ssr pass
    draw(0, 3, ssr_pass_data.tex_ssr_uv, 0);
}


void SSR::ssr_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ssr_pass_data.framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ssr_pass_data.size, ssr_pass_data.size);

    glBindTexture_(GL_TEXTURE_2D, 0, geometry_pass_data.tex_pos_view);
    glBindTexture_(GL_TEXTURE_2D, 1, geometry_pass_data.tex_normal_view);

    ssr_pass_data.shader.set_uniform({
            {"u_tex_pos_view", INT, {._int = 0}},
            {"u_tex_normal_view", INT, {._int = 1}},
            {"u_tex_size", VEC3, {._vec3 = {ssr_pass_data.size, ssr_pass_data.size, 0.f}}},
            {"u_camera_proj", MAT4, {._mat4 = Camera::proj_matrix()}},
    });
    model_square.mesh.draw();
}


void SSR::color_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport_({.width  = window.width,
                 .height = window.height,
                 .x_cnt  = 6,
                 .y_cnt  = 4,
                 .x_idx  = 2,
                 .y_idx  = 0,
                 .x_len  = 4,
                 .y_len  = 4});

    /// 绘制场景
    glm::mat3 view_it = glm::transpose(glm::inverse(glm::mat3(camera.view_matrix())));
    glBindTexture_(GL_TEXTURE_2D, 0, geometry_pass_data.tex_pos_view);
    glBindTexture_(GL_TEXTURE_2D, 1, geometry_pass_data.tex_normal_view);
    glBindTexture_(GL_TEXTURE_2D, 2, geometry_pass_data.tex_diffuse);
    glBindTexture_(GL_TEXTURE_2D, 3, light_pass_cfg.shadow_map);
    glBindTexture_(GL_TEXTURE_2D, 4, ssr_pass_data.tex_ssr_uv);
    shader.set_uniform({
            {"u_tex_pos_view", INT, {._int = 0}},
            {"u_tex_normal_view", INT, {._int = 1}},
            {"u_tex_diffuse", INT, {._int = 2}},
            {"u_tex_ssr_uv", INT, {._int = 4}},
            {"u_light_dir_view", VEC3, {._vec3 = view_it * light.light_dir()}},
            {"u_light_color", VEC3, {._vec3 = light.color}},
    });
    model_square.mesh.draw();

    /// 光源可视化
    {
        shader_diffuse.set_uniform({
                {shader_diffuse.m_proj, {._mat4 = Camera::proj_matrix()}},
                {shader_diffuse.m_view, {._mat4 = camera.view_matrix()}},
        });
        glm::vec3 scale = {0.2f, 0.2f, 0.2f};
        glm::mat4 m1    = glm::scale(glm::translate(glm::one<glm::mat4>(), light.pos), scale);
        glm::mat4 m2    = glm::scale(glm::translate(glm::one<glm::mat4>(), light.target), scale);

        auto draw = [&](const glm::mat4 &mat) {
            shader_diffuse.set_uniform({
                    {shader_diffuse.has_diffuse, {._int = 0}},
                    {shader_diffuse.kd, {._vec3 = {0.9f, 0.9f, 0.9f}}},
                    {shader_diffuse.m_model, {._mat4 = mat}},
            });
            model_cube.mesh.draw();
        };
        draw(m1);
        draw(m2);
    }

    /// 参考轴
    {
        axis.draw(Camera::proj_matrix() * camera.view_matrix());
    }
}


void SSR::geometry_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, geometry_pass_data.framebuffer);
    glViewport(0, 0, geometry_pass_data.size, geometry_pass_data.size);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    geometry_pass_data.shader.set_uniform({
            {"u_view", MAT4, {._mat4 = camera.view_matrix()}},
            {"u_proj", MAT4, {._mat4 = Camera::proj_matrix()}},
    });

    for (auto &m: scene)
    {
        if (m.tex_diffuse.has)
            glBindTexture_(GL_TEXTURE_2D, 0, m.tex_diffuse.id);
        geometry_pass_data.shader.set_uniform({
                {"u_model", MAT4, {._mat4 = m.model_matrix()}},
                {"u_has_diffuse", INT, {._int = m.tex_diffuse.has}},
                {"u_kd", VEC3, {._vec3 = m.color_diffuse}},
                {"u_tex_diffuse", INT, {._int = 0}},
        });
        m.mesh.draw();
    }
}


void SSR::light_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, light_pass_cfg.framebuffer);
    glViewport(0, 0, light_pass_cfg.size, light_pass_cfg.size);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto &m: scene)
    {
        light_pass_cfg.shader.set_uniform({
                {"u_light_mvp", MAT4, {._mat4 = light.proj_matrix * light.view_matrix_get() * m.model_matrix()}},
        });
        m.mesh.draw();
    }
}