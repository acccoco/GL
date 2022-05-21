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
#include "shader/tex2d-visual/tex-visual.h"
#include "shader/diffuse/diffuse.h"

const std::string CUR = EXAMPLES + "SSR/";

class ShaderShadowPass : public Shader
{
public:
    UniformAttribute u_light_mvp = {"u_light_mvp", this, MAT4};

    ShaderShadowPass()
        : Shader(CUR + "shadow-pass.vert", CUR + "shadow-pass.frag")
    {
        this->uniform_attrs_location_init();
    }
};

class ShaderGeometryPass : public Shader
{
public:
    UniformAttribute u_camera_vp = {"u_camera_vp", this, MAT4};

    UniformAttribute u_model       = {"u_model", this, MAT4};
    UniformAttribute u_has_diffuse = {"u_has_diffuse", this, INT};
    UniformAttribute u_kd          = {"u_kd", this, VEC3};
    UniformAttribute u_tex_diffse  = {"u_tex_diffuse", this, INT};

    UniformAttribute u_shadow_map  = {"u_shadow_map", this, INT};
    UniformAttribute u_light_dir   = {"u_light_dir", this, VEC3};
    UniformAttribute u_light_color = {"u_light_color", this, VEC3};
    UniformAttribute u_light_vp    = {"u_light_vp", this, MAT4};

    ShaderGeometryPass()
        : Shader(CUR + "geometry-pass.vert", CUR + "geometry-pass.frag")
    {
        this->uniform_attrs_location_init();
    }
};

class ShaderColorPass : public Shader
{
public:
    UniformAttribute u_camera_vp  = {"u_camera_vp", this, MAT4};
    UniformAttribute u_camera_vp_ = {"u_camera_vp_", this, MAT4};
    UniformAttribute u_camera_pos = {"u_camera_pos", this, VEC3};

    UniformAttribute u_model       = {"u_model", this, MAT4};
    UniformAttribute u_has_diffuse = {"u_has_diffuse", this, INT};
    UniformAttribute u_kd          = {"u_kd", this, VEC3};
    UniformAttribute u_tex_diffuse = {"u_tex_diffuse", this, INT};

    UniformAttribute u_tex_depth_visibility = {"u_tex_depth_visibility", this, INT};
    UniformAttribute u_tex_world_pos        = {"u_tex_world_pos", this, INT};
    UniformAttribute u_tex_world_normal     = {"u_tex_world_normal", this, INT};
    UniformAttribute u_tex_direct_color     = {"u_tex_direct_color", this, INT};

    ShaderColorPass()
        : Shader(CUR + "color-pass.vert", CUR + "color-pass.frag")
    {
        this->uniform_attrs_location_init();
    }
};


class SSR : public Engine
{
    struct {
        GLuint framebuffer{};
        GLuint shadow_map{};

        const GLsizei size = 512;
    } shadow_pass_fbo;
    /// 初始化 shadow pass 用到的 framebuffer 和纹理等
    void shadow_pass_init();

    struct {
        GLuint framebuffer{};
        GLuint tex_position{};            // layout 0
        GLuint tex_normal{};              // layout 1
        GLuint tex_depth_visibility{};    // layout 2  R 通道是深度，G 通道是 visibility
        GLuint tex_direct_color{};        // layout 3

        // todo 这个应该和 window size 一致，优化一下，而不是硬编码
        const GLsizei size = 1600;
    } geometry_pass_fbo;
    /// 初始化 geometry pass 用到的 framebuffer 和纹理等
    void geometry_pass_init();

    /// 场景中的方向光
    struct {
        glm::vec3 pos    = {0.f, 4.f, -2.f};
        glm::vec3 target = {0.f, 0.f, 0.f};
        glm::vec3 color  = {0.7f, 0.7f, 0.7f};

        /// 这里是正交投影！！！！
        const glm::mat4 proj_matrix = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1e-2f, 100.f);

        [[nodiscard]] glm::mat4 view_matrix_get() const { return glm::lookAt(pos, target, POSITIVE_Y); }
    } light;

    std::vector<Model> scene;
    std::vector<Model> model_three   = Model::load_obj(MODEL_THREE_OBJS);
    std::vector<Model> model_cornell = Model::load_obj(MODEL_CORNELL_BOX);

    ShaderShadowPass   shader_s_pass;
    ShaderGeometryPass shader_g_pass;
    ShaderColorPass    shader_c_pass;

    /// 光源可视化
    ShaderDiffuse shader_diffuse;
    Model         model_cube = Model::load_obj(MODEL_CUBE)[0];

    /// 纹理可视化
    Model           model_square = Model::load_obj(MODEL_SQUARE)[0];
    ShaderTexVisual shader_tex_visual;

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
        glm::mat4 light_vp  = light.proj_matrix * light.view_matrix_get();
        glm::mat4 camera_vp = Camera::proj_matrix() * camera.view_matrix();

        shadow_pass(light_vp);
        geometry_pass(light_vp, camera_vp);
        color_pass(camera_vp);
        debug_pass();
    }

    void shadow_pass(const glm::mat4 &light_vp)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_pass_fbo.framebuffer);
        glViewport(0, 0, shadow_pass_fbo.size, shadow_pass_fbo.size);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto &m: scene)
        {
            shader_s_pass.set_uniform({
                    {shader_s_pass.u_light_mvp, {._mat4 = light_vp * m.model_matrix()}},
            });
            m.mesh.draw();
        }
    }

    void geometry_pass(const glm::mat4 &light_vp, const glm::mat4 &camera_vp)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, geometry_pass_fbo.framebuffer);
        glViewport(0, 0, geometry_pass_fbo.size, geometry_pass_fbo.size);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindTexture_(GL_TEXTURE_2D, 0, shadow_pass_fbo.shadow_map);
        shader_g_pass.set_uniform({
                {shader_g_pass.u_camera_vp, {._mat4 = camera_vp}},
                {shader_g_pass.u_shadow_map, {._int = 0}},
                {shader_g_pass.u_light_dir, {._vec3 = light.target - light.pos}},
                {shader_g_pass.u_light_color, {._vec3 = light.color}},
                {shader_g_pass.u_light_vp, {._mat4 = light_vp}},
        });

        for (auto &m: scene)
        {
            if (m.tex_diffuse.has)
                glBindTexture_(GL_TEXTURE_2D, 1, m.tex_diffuse.id);
            shader_g_pass.set_uniform({
                    {shader_g_pass.u_model, {._mat4 = m.model_matrix()}},
                    {shader_g_pass.u_has_diffuse, {._int = m.tex_diffuse.has}},
                    {shader_g_pass.u_kd, {._vec3 = m.color_diffuse}},
                    {shader_g_pass.u_tex_diffse, {._int = 1}},
            });
            m.mesh.draw();
        }
    }

    void color_pass(const glm::mat4 &camera_vp)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport_({
                .width  = 1200,
                .height = 800,
                .x_cnt  = 6,
                .y_cnt  = 4,
                .x_idx  = 0,
                .y_idx  = 0,
                .x_len  = 4,
                .y_len  = 4,
        });

        glBindTexture_(GL_TEXTURE_2D, 0, geometry_pass_fbo.tex_depth_visibility);
        glBindTexture_(GL_TEXTURE_2D, 1, geometry_pass_fbo.tex_position);
        glBindTexture_(GL_TEXTURE_2D, 2, geometry_pass_fbo.tex_normal);
        glBindTexture_(GL_TEXTURE_2D, 3, geometry_pass_fbo.tex_direct_color);
        shader_c_pass.set_uniform({
                {shader_c_pass.u_camera_vp, {._mat4 = camera_vp}},
                {shader_c_pass.u_camera_vp_, {._mat4 = camera_vp}},
                {shader_c_pass.u_camera_pos, {._vec3 = camera.get_pos()}},
                {shader_c_pass.u_tex_depth_visibility, {._int = 0}},
                {shader_c_pass.u_tex_world_pos, {._int = 1}},
                {shader_c_pass.u_tex_world_normal, {._int = 2}},
                {shader_c_pass.u_tex_direct_color, {._int = 3}},
        });

        for (auto &m: scene)
        {
            if (m.tex_diffuse.has)
                glBindTexture_(GL_TEXTURE_2D, 4, m.tex_diffuse.id);
            shader_c_pass.set_uniform({
                    {shader_c_pass.u_model, {._mat4 = m.model_matrix()}},
                    {shader_c_pass.u_has_diffuse, {._int = m.tex_diffuse.has}},
                    {shader_c_pass.u_kd, {._vec3 = m.color_diffuse}},
                    {shader_c_pass.u_tex_diffuse, {._int = 4}},
            });
            m.mesh.draw();
        }

        /// 光源可视化
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
        draw(4, 3, shadow_pass_fbo.shadow_map, 1);

        /// geometry pass - position
        draw(4, 2, geometry_pass_fbo.tex_position, 0);

        /// geometry pass - normal
        draw(5, 2, geometry_pass_fbo.tex_normal, 0);

        /// geometry pass - depth
        draw(4, 1, geometry_pass_fbo.tex_depth_visibility, 1);

        /// geometry pass - direct color
        draw(5, 1, geometry_pass_fbo.tex_direct_color, 0);

        /// geometry pass - visibility
        draw(4, 0, geometry_pass_fbo.tex_depth_visibility, 2);
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
    glGenFramebuffers(1, &shadow_pass_fbo.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_pass_fbo.framebuffer);

    GLuint depth_buffer = create_depth_buffer(shadow_pass_fbo.size, shadow_pass_fbo.size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

    shadow_pass_fbo.shadow_map = new_tex2d({
            .width           = shadow_pass_fbo.size,
            .height          = shadow_pass_fbo.size,
            .internal_format = GL_R32F,
            .external_format = GL_RED,
            .external_type   = GL_FLOAT,
    });
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadow_pass_fbo.shadow_map, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        SPDLOG_ERROR("shadow pass framebuffer incomplete.");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void SSR::geometry_pass_init()
{
    glGenFramebuffers(1, &geometry_pass_fbo.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, geometry_pass_fbo.framebuffer);

    GLuint depth_buffer = create_depth_buffer(geometry_pass_fbo.size, geometry_pass_fbo.size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

    auto tex2d_info = Tex2DInfo{
            .width           = geometry_pass_fbo.size,
            .height          = geometry_pass_fbo.size,
            .internal_format = GL_RGBA32F,
            .external_format = GL_RGBA,
            .external_type   = GL_FLOAT,
    };
    geometry_pass_fbo.tex_position         = new_tex2d(tex2d_info);
    geometry_pass_fbo.tex_normal           = new_tex2d(tex2d_info);
    geometry_pass_fbo.tex_depth_visibility = new_tex2d(tex2d_info);
    geometry_pass_fbo.tex_direct_color     = new_tex2d(tex2d_info);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, geometry_pass_fbo.tex_position, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, geometry_pass_fbo.tex_normal, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, geometry_pass_fbo.tex_depth_visibility, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, geometry_pass_fbo.tex_direct_color, 0);

    /// multi render target
    GLenum layout_frag_out[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, layout_frag_out);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        SPDLOG_ERROR("geometry pass framebuffer incomplete.");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
