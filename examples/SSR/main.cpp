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

const std::string CUR        = EXAMPLES + "SSR/";
const std::string CUR_SHADER = CUR + "shader/";


class SSR : public Engine
{
    /// shadow pass 需要的数据
    struct {
        GLuint framebuffer{};
        GLuint shadow_map{};

        const GLsizei size = 512;
    } shadow_pass_data;

    /// 初始化 shadow pass 用到的 framebuffer 和纹理等
    void shadow_pass_init();

    /// color pass 需要的数据
    struct {
        GLuint framebuffer{};
        GLuint tex_position{};            // layout 0
        GLuint tex_normal{};              // layout 1
        GLuint tex_depth_visibility{};    // layout 2  R 通道是深度，G 通道是 visibility
        GLuint tex_direct_color{};        // layout 3

        // FIXME 这个应该和 window size 一致，优化一下，而不是硬编码
        const GLsizei size = 1600;
    } geometry_pass_data;

    /// 初始化 geometry pass 用到的 framebuffer 和纹理等
    void geometry_pass_init();

    /// 场景中的方向光
    struct {
        glm::vec3 pos    = {2.f, 7.f, 7.f};
        glm::vec3 target = {-1.f, 0.f, 0.f};
        glm::vec3 color  = {0.7f, 0.7f, 0.7f};

        /// 这里是正交投影！！！！
        const glm::mat4 proj_matrix = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1e-2f, 100.f);

        [[nodiscard]] glm::mat4 view_matrix_get() const { return glm::lookAt(pos, target, POSITIVE_Y); }
    } light;

    std::vector<Model> scene;
    std::vector<Model> model_three   = Model::load_obj(MODEL_THREE_OBJS);
    std::vector<Model> model_cornell = Model::load_obj(MODEL_CORNELL_BOX);

    Shader2 shader_shadow_pass   = Shader2(CUR_SHADER + "shadow-pass.vert", CUR_SHADER + "shadow-pass.frag");
    Shader2 shader_geometry_pass = Shader2(CUR_SHADER + "geometry-pass.vert", CUR_SHADER + "geometry-pass.frag");
    Shader2 shader_color_pass    = Shader2(CUR_SHADER + "color-pass.vert", CUR_SHADER + "color-pass.frag");

    /// 光源可视化
    ShaderDiffuse shader_diffuse;
    Model         model_cube = Model::load_obj(MODEL_CUBE)[0];

    /// 纹理可视化
    Model           model_square = Model::load_obj(MODEL_SQUARE)[0];
    ShaderTexVisual shader_tex_visual;

    // 坐标轴
    Axis axis;

public:
    void init() override
    {
        this->shadow_pass_init();
        this->geometry_pass_init();

        /// 在 mac 中，对应的实际尺寸是 2400 * 1600
        glfwSetWindowSize(window.ptr, 1200, 800);

        scene = model_three;
    }

    void tick_render() override
    {
        glClearColor(0.f, 0.f, 0.f, 0.f);

        glm::mat4 light_vp  = light.proj_matrix * light.view_matrix_get();
        glm::mat4 camera_vp = Camera::proj_matrix() * camera.view_matrix();

        shadow_pass(light_vp);
        geometry_pass(light_vp, camera_vp);
        color_pass(camera_vp);
        debug_pass();
    }

    void shadow_pass(const glm::mat4 &light_vp)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_pass_data.framebuffer);
        glViewport(0, 0, shadow_pass_data.size, shadow_pass_data.size);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto &m: scene)
        {
            shader_shadow_pass.set_uniform({
                    {"u_light_mvp", MAT4, {._mat4 = light_vp * m.model_matrix()}},
            });
            m.mesh.draw();
        }
    }

    void geometry_pass(const glm::mat4 &light_vp, const glm::mat4 &camera_vp)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, geometry_pass_data.framebuffer);
        glViewport(0, 0, geometry_pass_data.size, geometry_pass_data.size);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindTexture_(GL_TEXTURE_2D, 0, shadow_pass_data.shadow_map);
        shader_geometry_pass.set_uniform({
                {"u_camera_vp", MAT4, {._mat4 = camera_vp}},
                {"u_shadow_map", INT, {._int = 0}},
                {"u_light_dir", VEC3, {._vec3 = light.target - light.pos}},
                {"u_light_color", VEC3, {._vec3 = light.color}},
                {"u_light_vp", MAT4, {._mat4 = light_vp}},
        });

        for (auto &m: scene)
        {
            if (m.tex_diffuse.has)
                glBindTexture_(GL_TEXTURE_2D, 1, m.tex_diffuse.id);
            shader_geometry_pass.set_uniform({
                    {"u_model", MAT4, {._mat4 = m.model_matrix()}},
                    {"u_has_diffuse", INT, {._int = m.tex_diffuse.has}},
                    {"u_kd", VEC3, {._vec3 = m.color_diffuse}},
                    {"u_tex_diffuse", INT, {._int = 1}},
            });
            m.mesh.draw();
        }
    }

    void color_pass(const glm::mat4 &camera_vp)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport_(
                {.width = 1200, .height = 800, .x_cnt = 6, .y_cnt = 4, .x_idx = 0, .y_idx = 0, .x_len = 4, .y_len = 4});

        glBindTexture_(GL_TEXTURE_2D, 0, geometry_pass_data.tex_depth_visibility);
        glBindTexture_(GL_TEXTURE_2D, 1, geometry_pass_data.tex_position);
        glBindTexture_(GL_TEXTURE_2D, 2, geometry_pass_data.tex_normal);
        glBindTexture_(GL_TEXTURE_2D, 3, geometry_pass_data.tex_direct_color);
        shader_color_pass.set_uniform({
                {"u_camera_vp", MAT4, {._mat4 = camera_vp}},
                {"u_camera_vp_", MAT4, {._mat4 = camera_vp}},
                {"u_camera_pos", VEC3, {._vec3 = camera.get_pos()}},
                {"u_tex_size", VEC3, {._vec3 = {geometry_pass_data.size, geometry_pass_data.size, 0.f}}},
                {"u_tex_world_pos", INT, {._int = 1}},
                {"u_tex_world_normal", INT, {._int = 2}},
                {"u_tex_direct_color", INT, {._int = 3}},
        });

        for (auto &m: scene)
        {
            if (m.tex_diffuse.has)
                glBindTexture_(GL_TEXTURE_2D, 4, m.tex_diffuse.id);
            shader_color_pass.set_uniform({
                    {"u_model", MAT4, {._mat4 = m.model_matrix()}},
                    {"u_has_diffuse", INT, {._int = m.tex_diffuse.has}},
                    {"u_kd", VEC3, {._vec3 = m.color_diffuse}},
                    {"u_tex_diffuse", INT, {._int = 4}},
            });
            m.mesh.draw();
        }

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
            axis.draw(camera_vp);
        }
    }

    void debug_pass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ViewPortInfo viewport_info = {
                .width  = 1200,
                .height = 800,
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

        /// shadow pass - shadow map
        draw(4, 3, shadow_pass_data.shadow_map, 1);

        /// geometry pass - position
        draw(4, 2, geometry_pass_data.tex_position, 0);

        /// geometry pass - normal
        draw(5, 2, geometry_pass_data.tex_normal, 0);

        /// geometry pass - depth
        draw(4, 1, geometry_pass_data.tex_depth_visibility, 1);

        /// geometry pass - direct color
        draw(5, 1, geometry_pass_data.tex_direct_color, 0);

        /// geometry pass - visibility
        draw(4, 0, geometry_pass_data.tex_depth_visibility, 2);
    }


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
    auto a = SSR();
    a.engine_main();
}


/// ==================================================================


void SSR::shadow_pass_init()
{
    glGenFramebuffers(1, &shadow_pass_data.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_pass_data.framebuffer);

    GLuint depth_buffer = create_depth_buffer(shadow_pass_data.size, shadow_pass_data.size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

    shadow_pass_data.shadow_map = new_tex2d({
            .width           = shadow_pass_data.size,
            .height          = shadow_pass_data.size,
            .internal_format = GL_R32F,
            .external_format = GL_RED,
            .external_type   = GL_FLOAT,
    });
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadow_pass_data.shadow_map, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        SPDLOG_ERROR("shadow pass framebuffer incomplete.");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void SSR::geometry_pass_init()
{
    glGenFramebuffers(1, &geometry_pass_data.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, geometry_pass_data.framebuffer);

    GLuint depth_buffer = create_depth_buffer(geometry_pass_data.size, geometry_pass_data.size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

    auto tex2d_info = Tex2DInfo{
            .width           = geometry_pass_data.size,
            .height          = geometry_pass_data.size,
            .internal_format = GL_RGBA32F,
            .external_format = GL_RGBA,
            .external_type   = GL_FLOAT,
    };
    geometry_pass_data.tex_position         = new_tex2d(tex2d_info);
    geometry_pass_data.tex_normal           = new_tex2d(tex2d_info);
    geometry_pass_data.tex_depth_visibility = new_tex2d(tex2d_info);
    geometry_pass_data.tex_direct_color     = new_tex2d(tex2d_info);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, geometry_pass_data.tex_position, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, geometry_pass_data.tex_normal, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, geometry_pass_data.tex_depth_visibility, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, geometry_pass_data.tex_direct_color, 0);

    /// multi render target
    GLenum layout_frag_out[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, layout_frag_out);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        SPDLOG_ERROR("geometry pass framebuffer incomplete.");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
