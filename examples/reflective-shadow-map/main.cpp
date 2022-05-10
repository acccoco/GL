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
#include "core/model.h"
#include "core/misc.h"
#include "shader/tex2d-visual/tex-visual.h"


class ShaderGenRSM : public Shader
{
public:
    UniformAttributeM4fv m_model         = {"m_model", this};
    UniformAttributeM4fv m_view          = {"m_view", this};
    UniformAttributeM4fv m_proj          = {"m_proj", this};
    UniformAttribute3fv  light_indensity = {"light_indensity", this};
    UniformAttribute1f   fov_deg         = {"fov_deg", this};
    UniformAttribute1f   viewport_size   = {"viewport_size", this};
    UniformAttribute3fv  kd              = {"kd", this};
    UniformAttribute1i   has_diffuse     = {"has_diffuse", this};
    UniformAttribute1i   tex_diffuse     = {"tex_diffuse", this};
    UniformAttribute1f   zoom_in         = {"zoom_in", this};

    ShaderGenRSM()
        : Shader(EXAMPLES + "reflective-shadow-map/gen-rsm.vert", EXAMPLES + "reflective-shadow-map/gen-rsm.frag")
    {
        this->uniform_attrs_location_init();
    }

    /**
     * @param fov 摄像机视角，degree
     * @param near 近平面长宽
     * @param zoom_in_ flux 放大倍数的 sqrt
     */
    void init(float fov, float near, const glm::mat4 &proj, float zoom_in_);

    /**
     * @param indensity 光源的 Indensity
     */
    void update_per_frame(const glm::mat4 &view, const glm::vec3 &indensity);

    void draw_model(const Model &m);
};


class ShaderGI : public Shader
{
public:
    UniformAttributeM4fv m_model      = {"m_model", this};
    UniformAttributeM4fv m_view       = {"m_view", this};
    UniformAttributeM4fv m_proj       = {"m_proj", this};
    UniformAttributeM4fv light_VP     = {"light_VP", this};
    UniformAttribute1i   RSM_pos      = {"RSM_pos", this};
    UniformAttribute1i   RSM_normal   = {"RSM_normal", this};
    UniformAttribute1i   RSM_flux     = {"RSM_flux", this};
    UniformAttribute1f   flux_zoom_in = {"flux_zoom_in", this};
    UniformAttribute3fv  random_seed  = {"random_seed", this};
    UniformAttribute3fv  light_pos    = {"light_pos", this};
    UniformAttribute1i   has_diffuse  = {"has_diffuse", this};
    UniformAttribute1i   tex_diffuse  = {"tex_diffuse", this};
    UniformAttribute3fv  kd           = {"kd", this};

    ShaderGI()
        : Shader(EXAMPLES + "reflective-shadow-map/gi.vert", EXAMPLES + "reflective-shadow-map/gi.frag")
    {
        this->uniform_attrs_location_init();
    }

    void init(const glm::mat4 &proj, float zoom_in);

    void update_per_frame(const glm::mat4 &view, const glm::mat4 &VP_light, const glm::vec3 &light_pos_,
                          const glm::vec3 &seed, GLuint tex_pos, GLuint tex_normal, GLuint tex_flux);

    void draw(const Model &model);
};

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

    ShaderGenRSM shader;

    RSMFrameBuffer();

    /// 创建一张 4 通道的 2D 纹理，精度为 32f
    static GLuint create_tex_2d(GLsizei size);

    /**
     * 将场景信息渲染到 RSM 中
     * @param light_pos 光源的位置。光源默认是朝向原点的
     * @param indensity 光源的 Indensity
     */
    void gen_RSM(glm::vec3 light_pos, glm::vec3 indensity, const std::vector<Model> &models);
};


class RSM : public Engine
{
    RSMFrameBuffer  fbo;
    ShaderTexVisual tex_visual;
    Model           model_square = Model::load_obj(MODEL_SQUARE)[0];

    /// light 固定朝向原点
    struct {
        glm::vec3 pos       = {0.85f, 1.90f, 0.76f};
        glm::vec3 indensity = glm::vec3{1.f, 1.f, 1.f} * 0.004f;
    } light;

    std::vector<Model> model_bunny  = Model::load_obj(MODEL_BUNNY);
    std::vector<Model> model_cornel = Model::load_obj(MODEL_CORNER);
    std::vector<Model> models;

    ShaderGI shader_gi;

    void init() override
    {
        model_bunny[0].pos = {0.05f, -0.2f, -0.25f};
        models.insert(models.end(), model_bunny.begin(), model_bunny.end());
        models.insert(models.end(), model_cornel.begin(), model_cornel.end());

        shader_gi.init(Camera::proj_matrix(), fbo.zoom_in);
    }

    void tick_pre_render() override
    {
        fbo.gen_RSM(light.pos, light.indensity, models);
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport_(window.width, window.height, 4, 4, 3, 3);
        tex_visual.draw(model_square, fbo.tex_pos);

        glViewport_(window.width, window.height, 4, 4, 3, 2);
        tex_visual.draw(model_square, fbo.tex_normal);

        glViewport_(window.width, window.height, 4, 4, 3, 1);
        tex_visual.draw(model_square, fbo.tex_flux);

        glViewport_(window.width, window.height, 4, 4, 0, 0, 3, 3);
        auto light_V = glm::lookAt(light.pos, glm::vec3(0.f), POSITIVE_Y);
        auto light_P = glm::perspective(fbo.fov, 1.f, fbo.near, fbo.far);
        // fixme 随机数种子
        shader_gi.update_per_frame(camera.view_matrix(), light_P * light_V, light.pos, glm::vec3(0.5f), fbo.tex_pos,
                                   fbo.tex_normal, fbo.tex_flux);
        for (auto &m: models)
            shader_gi.draw(m);
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);
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
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer);

    /// create and attach color attachment
    tex_pos    = create_tex_2d(SIZE);
    tex_normal = create_tex_2d(SIZE);
    tex_flux   = create_tex_2d(SIZE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_pos, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tex_normal, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, tex_flux, 0);

    /// set multi render target
    GLenum frag_out_locations[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, frag_out_locations);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        SPDLOG_ERROR("frame buffer uncomplete.");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /// init shader
    shader.init(fov, near, glm::perspective(fov, 1.f, near, far), zoom_in);
}
void RSMFrameBuffer::gen_RSM(const glm::vec3 light_pos, const glm::vec3 indensity, const std::vector<Model> &models)
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glViewport(0, 0, SIZE, SIZE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.update_per_frame(glm::lookAt(light_pos, glm::vec3(0.f), POSITIVE_Y), indensity);

    for (auto &m: models)
        shader.draw_model(m);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShaderGenRSM::init(float fov, float near, const glm::mat4 &proj, float zoom_in_)
{
    this->fov_deg.set(fov);
    this->viewport_size.set(near);
    this->m_proj.set(proj);
    this->zoom_in.set(zoom_in_);
}

void ShaderGenRSM::update_per_frame(const glm::mat4 &view, const glm::vec3 &indensity)
{
    this->m_view.set(view);
    this->light_indensity.set(indensity);
}

void ShaderGenRSM::draw_model(const Model &m)
{
    this->kd.set(m.color_diffuse);
    this->has_diffuse.set(m.tex_diffuse.has);
    this->m_model.set(m.model_matrix());
    if (m.tex_diffuse.has)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m.tex_diffuse.id);
    }
    this->tex_diffuse.set(0);
    this->use();
    m.mesh.draw();
}


void ShaderGI::init(const glm::mat4 &proj, float zoom_in)
{
    m_proj.set(proj);
    flux_zoom_in.set(zoom_in);
}
void ShaderGI::update_per_frame(const glm::mat4 &view, const glm::mat4 &VP_light, const glm::vec3 &light_pos_,
                                const glm::vec3 &seed, GLuint tex_pos, GLuint tex_normal, GLuint tex_flux)
{
    m_view.set(view);
    light_VP.set(VP_light);
    light_pos.set(light_pos_);
    random_seed.set(seed);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_pos);
    RSM_pos.set(1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_normal);
    RSM_normal.set(2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, tex_flux);
    RSM_flux.set(3);
}


void ShaderGI::draw(const Model &model)
{
    m_model.set(model.model_matrix());
    has_diffuse.set(model.tex_diffuse.has);
    kd.set(model.color_diffuse);
    if (model.tex_diffuse.has)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model.tex_diffuse.id);
    }
    tex_diffuse.set(0);
    this->use();
    model.mesh.draw();
}
