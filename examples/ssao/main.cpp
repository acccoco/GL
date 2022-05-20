#include <random>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/model.h"
#include "core/shader.h"
#include "core/misc.h"
#include "shader/tex2d-visual/tex-visual.h"


/// 生成场景几何信息的着色器：法线，深度信息
class ShaderGeometry : public Shader
{
public:
    UniformAttribute m_model = {"m_model", this, UniAttrType::MAT4};
    UniformAttribute m_view  = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj  = {"m_proj", this, UniAttrType::MAT4};

    ShaderGeometry()
        : Shader(EXAMPLES + "ssao/geometry.vert", EXAMPLES + "ssao/geometry.frag")
    {
        this->uniform_attrs_location_init();
    }
};


class ShaderSSAO : public Shader
{
public:
    UniformAttribute m_model   = {"m_model", this, UniAttrType::MAT4};
    UniformAttribute m_view    = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj    = {"m_proj", this, UniAttrType::MAT4};
    UniformAttribute camera_vp = {"camera_vp", this, UniAttrType::MAT4};

    UniformAttribute tex_geometry = {"tex_geometry", this, UniAttrType::INT};

    UniformAttribute has_diffuse = {"has_diffuse", this, UniAttrType::INT};
    UniformAttribute tex_diffuse = {"tex_diffuse", this, UniAttrType::INT};
    UniformAttribute kd          = {"kd", this, UniAttrType::VEC3};

    UniformAttribute light_pos   = {"light_pos", this, UniAttrType::VEC3};
    UniformAttribute rand_seed3  = {"rand_seed3", this, UniAttrType::VEC3};
    UniformAttribute ssao_on     = {"ssao_on", this, UniAttrType::INT};
    UniformAttribute ssao_radius = {"ssao_radius", this, UniAttrType::FLOAT};
    UniformAttribute ssao_only   = {"ssao_only", this, UniAttrType::INT};

    ShaderSSAO()
        : Shader(EXAMPLES + "ssao/ssao.vert", EXAMPLES + "ssao/ssao.frag")
    {
        this->uniform_attrs_location_init();
    }
};


class SSAO : public Engine
{
    /// first pass 相关的组件
    GLuint        tex_geometry{};
    GLuint        frame_buffer{};
    GLuint        depth_buffer{};
    const GLsizei framebuffer_size = 512;
    bool          ssao_on          = true;
    float         ssao_radius      = 0.2f;
    bool          ssao_only        = false;

    ShaderGeometry  shader_geometry;
    ShaderSSAO      shader_ssao;
    ShaderTexVisual shader_texvisual;

    std::vector<Model> model_three  = Model::load_obj(MODEL_THREE_OBJS);
    Model              model_square = Model::load_obj(MODEL_SQUARE)[0];
    std::vector<Model> model_lucy   = Model::load_obj(MODEL_LUCY);


    void init() override
    {
        /// framebuffer
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        /// framebuffer: depth attachment
        depth_buffer = create_depth_buffer(framebuffer_size, framebuffer_size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

        /// framebuffer: color attachment
        tex_geometry = new_tex2d({
                .width           = framebuffer_size,
                .height          = framebuffer_size,
                .internal_format = GL_RGBA32F,
                .external_format = GL_RGBA,
                .external_type   = GL_FLOAT,
                .wrap            = GL_REPEAT,
                .filter          = GL_LINEAR,
        });
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_geometry, 0);
    }

    void tick_pre_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        glViewport(0, 0, framebuffer_size, framebuffer_size);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /// set shader uniform attributes
        shader_geometry.set_uniform({
                {shader_geometry.m_proj, {._mat4 = Camera::proj_matrix()}},
                {shader_geometry.m_view, {._mat4 = camera.view_matrix()}},
        });

        /// draw meshes
        for (auto &m: model_lucy)
        {
            shader_geometry.set_uniform({
                    {shader_geometry.m_model, {._mat4 = m.model_matrix()}},
            });
            m.mesh.draw();
        }
    }

    void tick_render() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        static std::random_device             rd;
        static std::mt19937                   gen(rd());
        std::uniform_real_distribution<float> ur(0.f, 1.f);

        /// debug depth
        glViewport_(window.width, window.height, 3, 3, 2, 2);
        shader_texvisual.set_uniform({
                {shader_texvisual.channel, {._int = 0}},
        });
        shader_texvisual.draw(model_square, tex_geometry);


        /// draw scene use SSAO
        glViewport_(0, 0, window.width, window.height);

        glBindTexture_(GL_TEXTURE_2D, 0, tex_geometry);
        shader_ssao.set_uniform({
                {shader_ssao.m_proj, {._mat4 = Camera::proj_matrix()}},
                {shader_ssao.m_view, {._mat4 = camera.view_matrix()}},
                {shader_ssao.tex_geometry, {._int = 0}},
                {shader_ssao.camera_vp, {._mat4 = Camera::proj_matrix() * camera.view_matrix()}},
                {shader_ssao.light_pos, {._vec3 = glm::vec3(-1.f, 2.f, 3.f)}},
                {shader_ssao.rand_seed3, {._vec3 = glm::vec3(ur(gen), ur(gen), 0.f)}},
                {shader_ssao.ssao_on, {._int = ssao_on}},
                {shader_ssao.ssao_radius, {._float = ssao_radius}},
                {shader_ssao.ssao_only, {._int = ssao_only}},
        });

        for (auto &m: model_lucy)
        {
            if (m.tex_diffuse.has)
                glBindTexture_(GL_TEXTURE_2D, 1, m.tex_diffuse.id);
            shader_ssao.set_uniform({
                    {shader_ssao.m_model, {._mat4 = m.model_matrix()}},
                    {shader_ssao.has_diffuse, {._int = m.tex_diffuse.has}},
                    {shader_ssao.tex_diffuse, {._int = 1}},
                    {shader_ssao.kd, {._vec3 = m.color_diffuse}},
            });
            m.mesh.draw();
        }
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);
        ImGui::SliderFloat("ssao radius", &ssao_radius, 0.f, 2.f);
        ImGui::Checkbox("ssao on", &ssao_on);
        ImGui::Checkbox("ssao only", &ssao_only);
        ImGui::End();
    }
};


int main()
{
    auto engine = SSAO();
    engine.engine_main();
}