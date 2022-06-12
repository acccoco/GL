#include <random>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/shader.h"
#include "shader/tex2d-visual/tex-visual.h"



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

    /// 生成场景几何信息的着色器：法线，深度信息
    Shader2         shader_geometry = {EXAMPLE_CUR_PATH + "shader/geometry.vert",
                                       EXAMPLE_CUR_PATH + "shader/geometry.frag"};
    Shader2         shader_ssao     = {EXAMPLE_CUR_PATH + "shader/ssao.vert",
                                       EXAMPLE_CUR_PATH + "shader/ssao.frag"};
    ShaderTexVisual shader_texvisual;

    std::vector<RTObject> model_three  = ImportObj::load_obj(MODEL_THREE_OBJS);
    RTObject              model_square = ImportObj::load_obj(MODEL_SQUARE)[0];
    std::vector<RTObject> model_lucy   = ImportObj::load_obj(MODEL_LUCY);


    void init() override
    {
        /// framebuffer
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        /// framebuffer: depth attachment
        depth_buffer = create_depth_buffer(framebuffer_size, framebuffer_size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  depth_buffer);

        /// framebuffer: color attachment
        tex_geometry = new_tex2d(Tex2DInfo{
                .width           = framebuffer_size,
                .height          = framebuffer_size,
                .internal_format = GL_RGBA32F,
                .external_format = GL_RGBA,
                .external_type   = GL_FLOAT,
                .wrap_s          = GL_REPEAT,
                .wrap_t          = GL_REPEAT,
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
                {"m_proj", camera.proj_matrix()},
                {"m_view", camera.view_matrix()},
        });

        /// draw meshes
        for (auto &m: model_lucy)
        {
            shader_geometry.set_uniform({{"m_model", m.matrix()}});
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
        glViewport_(Window::framebuffer_width(), Window::framebuffer_height(), 3, 3, 2, 2);
        shader_texvisual.draw(model_square, tex_geometry, 0);

        /// draw scene use SSAO
        glViewport(0, 0, Window::framebuffer_width(), Window::framebuffer_height());

        glBindTexture_(GL_TEXTURE_2D, 0, tex_geometry);
        shader_ssao.set_uniform({
                {"m_proj", camera.proj_matrix()},
                {"m_view", camera.view_matrix()},
                {"tex_geometry", 0},
                {"camera_vp", camera.proj_matrix() * camera.view_matrix()},
                {"light_pos", glm::vec3(-1.f, 2.f, 3.f)},
                {"rand_seed3", glm::vec3(ur(gen), ur(gen), 0.f)},
                {"ssao_on", ssao_on},
                {"ssao_radius", ssao_radius},
                {"ssao_only", ssao_only},
        });

        for (auto &m: model_lucy)
        {
            const Material & mat = m.mesh.mat;
            if (mat.has_tex_basecolor())
                glBindTexture_(GL_TEXTURE_2D, 1, mat.metallic_roughness.tex_base_color);
            shader_ssao.set_uniform({
                    {"m_model", m.matrix()},
                    {"has_diffuse", mat.has_tex_basecolor()},
                    {"tex_diffuse", 1},
                    {"kd", glm::vec3(mat.metallic_roughness.base_color)},
            });
            m.mesh.draw();
        }
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y,
                    camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw,
                    camera.get_euler().pitch);
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