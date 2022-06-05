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


const std::string cur_shader = EXAMPLE_CUR_PATH + "shader/";


class ShaderGenRSM : public Shader
{
public:
    UniformAttribute m_model = {"m_model", this, UniAttrType::MAT4};
    UniformAttribute m_view  = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj  = {"m_proj", this, UniAttrType::MAT4};

    UniformAttribute light_indensity = {"light_indensity", this, UniAttrType::VEC3};
    UniformAttribute fov_deg         = {"fov_deg", this, UniAttrType::FLOAT};
    UniformAttribute viewport_size   = {"viewport_size", this, UniAttrType::FLOAT};
    UniformAttribute kd              = {"kd", this, UniAttrType::VEC3};
    UniformAttribute has_diffuse     = {"has_diffuse", this, UniAttrType::INT};
    UniformAttribute tex_diffuse     = {"tex_diffuse", this, UniAttrType::INT};
    UniformAttribute zoom_in         = {"zoom_in", this, UniAttrType::FLOAT};

    ShaderGenRSM()
        : Shader(cur_shader + "gen-rsm.vert", cur_shader + "gen-rsm.frag")
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
    UniformAttribute m_model      = {"m_model", this, UniAttrType::MAT4};
    UniformAttribute m_view       = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj       = {"m_proj", this, UniAttrType::MAT4};
    UniformAttribute light_VP     = {"light_VP", this, UniAttrType::MAT4};
    UniformAttribute RSM_pos      = {"RSM_pos", this, UniAttrType::INT};
    UniformAttribute RSM_normal   = {"RSM_normal", this, UniAttrType::INT};
    UniformAttribute RSM_flux     = {"RSM_flux", this, UniAttrType::INT};
    UniformAttribute flux_zoom_in = {"flux_zoom_in", this, UniAttrType::FLOAT};
    UniformAttribute random_seed  = {"random_seed", this, UniAttrType::VEC3};
    UniformAttribute light_pos    = {"light_pos", this, UniAttrType::VEC3};
    UniformAttribute has_diffuse  = {"has_diffuse", this, UniAttrType::INT};
    UniformAttribute tex_diffuse  = {"tex_diffuse", this, UniAttrType::INT};
    UniformAttribute kd           = {"kd", this, UniAttrType::VEC3};

    ShaderGI()
        : Shader(cur_shader + "gi.vert", cur_shader + "gi.frag")
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

    void tick_pre_render() override { fbo.gen_RSM(light.pos, light.indensity, models); }

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
    this->set_uniform({
            {fov_deg, {._float = fov}},
            {viewport_size, {._float = near}},
            {m_proj, {._mat4 = proj}},
            {zoom_in, {._float = zoom_in_}},
    });
}

void ShaderGenRSM::update_per_frame(const glm::mat4 &view, const glm::vec3 &indensity)
{
    this->set_uniform({
            {m_view, {._mat4 = view}},
            {light_indensity, {._vec3 = indensity}},
    });
}

void ShaderGenRSM::draw_model(const Model &m)
{
    if (m.tex_diffuse.has)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m.tex_diffuse.id);
    }

    this->set_uniform({
            {kd, {._vec3 = m.color_diffuse}},
            {has_diffuse, {._int = m.tex_diffuse.has}},
            {m_model, {._mat4 = m.model_matrix()}},
            {tex_diffuse, {._int = 0}},
    });

    m.mesh.draw();
}


void ShaderGI::init(const glm::mat4 &proj, float zoom_in)
{
    this->set_uniform({
            {m_proj, {._mat4 = proj}},
            {flux_zoom_in, {._float = zoom_in}},
    });
}

void ShaderGI::update_per_frame(const glm::mat4 &view, const glm::mat4 &VP_light, const glm::vec3 &light_pos_,
                                const glm::vec3 &seed, GLuint tex_pos, GLuint tex_normal, GLuint tex_flux)
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_pos);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_normal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, tex_flux);

    this->set_uniform({
            {m_view, {._mat4 = view}},
            {light_VP, {._mat4 = VP_light}},
            {light_pos, {._vec3 = light_pos_}},
            {random_seed, {._vec3 = seed}},
            {RSM_pos, {._int = 1}},
            {RSM_normal, {._int = 2}},
            {RSM_flux, {._int = 3}},
    });
}


void ShaderGI::draw(const Model &model)
{
    if (model.tex_diffuse.has)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model.tex_diffuse.id);
    }
    this->set_uniform({
            {m_model, {._mat4 = model.model_matrix()}},
            {has_diffuse, {._int = model.tex_diffuse.has}},
            {kd, {._vec3 = model.color_diffuse}},
            {tex_diffuse, {._int = 0}},
    });
    model.mesh.draw();
}
