/**
 * RSM 实现全局光照
 *
 * 注：flux 放大了很多倍
 */

#include <array>
#include <vector>
#include <string>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/misc.h"
#include "core/rt-object.h"
#include "core/import-obj.h"
#include "shader/tex2d-visual/tex-visual.h"


const std::string cur_shader = EXAMPLE_CUR_PATH + "shader/";


/// 用于渲染 reflective shadow map 的 framebuffer
struct RSMFrameBuffer {
    const GLsizei SIZE    = 512;
    const float   near    = 0.1f;
    const float   far     = 10.f;
    const float   fov     = 90.f;
    const float   zoom_in = 10000.f;    // flux 放大的倍数

    GLuint id{};
    GLuint depth_render_buffer{};

    GLuint tex_pos;       // layout 0 位置信息，深度信息
    GLuint tex_normal;    // layout 1 法线信息
    GLuint tex_flux;      // layout 2 光照功率信息

    Shader2 shader = {cur_shader + "gen-rsm.vert", cur_shader + "gen-rsm.frag"};

    RSMFrameBuffer();

    /// 创建一张 4 通道的 2D 纹理，精度为 32f
    static GLuint create_tex_2d(GLsizei size);

    /**
     * 将场景信息渲染到 RSM 中
     * @param light_pos 光源的位置。光源默认是朝向原点的
     * @param indensity 光源的 Indensity
     */
    void gen_RSM(glm::vec3 light_pos, glm::vec3 indensity, const std::vector<RTObject> &models);
};


class RSM : public Engine
{
    RSMFrameBuffer  fbo;
    ShaderTexVisual tex_visual;
    RTObject        model_square = ImportObj::load_obj(MODEL_SQUARE)[0];

    /// light 固定朝向原点
    struct {
        glm::vec3 pos       = {0.85f, 1.90f, 0.76f};
        glm::vec3 indensity = glm::vec3{1.f, 1.f, 1.f} * 0.004f;
    } light;

    std::vector<RTObject> model_bunny  = ImportObj::load_obj(MODEL_BUNNY);
    std::vector<RTObject> model_cornel = ImportObj::load_obj(MODEL_CORNER);
    std::vector<RTObject> models;

    Shader2 shader_gi = {cur_shader + "gi.vert", cur_shader + "gi.frag"};

    void init() override
    {
        model_bunny[0].set_pos({0.05f, -0.2f, -0.25f});
        models.insert(models.end(), model_bunny.begin(), model_bunny.end());
        models.insert(models.end(), model_cornel.begin(), model_cornel.end());

        shader_gi.set_uniform({
                {"m_proj", camera.proj_matrix()},
                {"flux_zoom_in", fbo.zoom_in},
        });
    }

    void tick_pre_render() override { fbo.gen_RSM(light.pos, light.indensity, models); }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport_(Window::framebuffer_width(), Window::framebuffer_height(), 4, 4, 3, 3);
        tex_visual.draw(model_square, fbo.tex_pos);

        glViewport_(Window::framebuffer_width(), Window::framebuffer_height(), 4, 4, 3, 2);
        tex_visual.draw(model_square, fbo.tex_normal);

        glViewport_(Window::framebuffer_width(), Window::framebuffer_height(), 4, 4, 3, 1);
        tex_visual.draw(model_square, fbo.tex_flux);

        glViewport_(Window::framebuffer_width(), Window::framebuffer_height(), 4, 4, 0, 0, 3, 3);
        auto light_V = glm::lookAt(light.pos, glm::vec3(0.f), POSITIVE_Y);
        auto light_P = glm::perspective(fbo.fov, 1.f, fbo.near, fbo.far);

        // TODO 随机数种子
        shader_gi.set_uniform({
                {"m_view", camera.view_matrix()},
                {"light_VP", light_P * light_V},
                {"light_pos", light.pos},
                {"random_seed", glm::vec3(0.5f)},
                {"RSM_pos", 1},
                {"RSM_normal", 2},
                {"RSM_flux", 3},
        });
        glBindTexture_(GL_TEXTURE_2D, 1, fbo.tex_pos);
        glBindTexture_(GL_TEXTURE_2D, 2, fbo.tex_normal);
        glBindTexture_(GL_TEXTURE_2D, 3, fbo.tex_flux);

        for (auto &m: models)
        {
            const Material &mat = m.mesh.mat;
            if (mat.has_tex_basecolor())
                glBindTexture_(GL_TEXTURE_2D, 0, mat.metallic_roughness.tex_base_color);
            shader_gi.set_uniform({
                    {"m_model", m.matrix()},
                    {"has_diffuse", mat.has_tex_basecolor()},
                    {"kd", glm::vec3(mat.metallic_roughness.base_color)},
                    {"tex_diffuse", 0},
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
        ImGui::Text("light pos: (%.2f, %.2f, %.2f)", light.pos.x, light.pos.y, light.pos.z);
        ImGui::SliderFloat("light pos x", &light.pos.x, -10.f, 10.f);
        ImGui::SliderFloat("light pos y", &light.pos.y, -10.f, 10.f);
        ImGui::SliderFloat("light pos z", &light.pos.z, -10.f, 10.f);
        ImGui::End();
    }
};


int main()
{
    auto engine = RSM();
    engine.engine_main();
}


//////////////////////////
// implement
///////////////////


GLuint RSMFrameBuffer::create_tex_2d(GLsizei size)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //    float border[] = {0.f, 0.f, 0.f, 0.f};
    //    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

RSMFrameBuffer::RSMFrameBuffer()
{
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    /// create and attach depth attachment
    glGenRenderbuffers(1, &depth_render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SIZE, SIZE);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              depth_render_buffer);

    /// create and attach color attachment
    tex_pos    = create_tex_2d(SIZE);
    tex_normal = create_tex_2d(SIZE);
    tex_flux   = create_tex_2d(SIZE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_pos, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tex_normal, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, tex_flux, 0);

    /// set multi render target
    GLenum frag_out_locations[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                   GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, frag_out_locations);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        SPDLOG_ERROR("frame buffer uncomplete.");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /// init shader
    shader.set_uniform({
            {"fov_deg", fov},
            {"viewport_size", near},
            {"m_proj", glm::perspective(fov, 1.f, near, far)},
            {"zoom_in", zoom_in},
    });
}
void RSMFrameBuffer::gen_RSM(const glm::vec3 light_pos, const glm::vec3 indensity,
                             const std::vector<RTObject> &models)
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glViewport(0, 0, SIZE, SIZE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.set_uniform({
            {"m_view", glm::lookAt(light_pos, glm::vec3(0.f), POSITIVE_Y)},
            {"light_indensity", indensity},
    });

    for (auto &m: models)
    {
        const Material &mat = m.mesh.mat;
        if (mat.has_tex_basecolor())
            glBindTexture_(GL_TEXTURE_2D, 0, mat.metallic_roughness.tex_base_color);
        shader.set_uniform({
                {"kd", glm::vec3(mat.metallic_roughness.base_color)},
                {"has_diffuse", mat.has_tex_basecolor()},
                {"m_model", m.matrix()},
                {"tex_diffuse", 0},
        });
        m.mesh.draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
