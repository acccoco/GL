/**
 * @brief 环境光照，使用 split sum approximate 实现 specular 光照的计算
 * @ref Real Shading in Unreal Engine 4 - Wei Zhi
 * @ref LearnOpenGL: https://learnopengl.com/PBR/IBL/Specular-IBL
 * @ref GAMES202: Real-Time Environment Mapping
 */

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/model.h"
#include "core/misc.h"

#include "shader/skybox/skybox.h"
#include "shader/diffuse/diffuse.h"
#include "./shader.h"

struct SplitSumApproximate {
    GLuint frame_buffer{};
    GLuint depth_render_buffer{};

    GLuint                filtered_env_map;
    const GLsizei         CUBE_SIZE             = 256;
    const GLint           TOTAL_CUBE_MIP_LEVELS = 6;
    ShaderPrefilterEnvMap shader_envmap;
    Model                 model_cube = Model::load_obj(MODEL_CUBE)[0];

    GLuint        brdf_lut{};
    const GLsizei LUT_SIZE     = 512;
    Model         model_square = Model::load_obj(MODEL_SQUARE)[0];
    ShaderIntBRDF shader_int_brdf;

    SplitSumApproximate()
    {
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        glGenRenderbuffers(1, &depth_render_buffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer);

        filtered_env_map = new_cubemap({
                .size            = CUBE_SIZE,
                .internal_format = GL_RGB32F,
                .external_format = GL_RGB,
                .external_type   = GL_FLOAT,
                // first linear: in a level; second linear: between level
                .min_filter = GL_LINEAR_MIPMAP_LINEAR,
                .mip_map    = true,
        });

        brdf_lut = new_tex2d({
                .width           = LUT_SIZE,
                .height          = LUT_SIZE,
                .internal_format = GL_RG32F,
                .external_format = GL_RG,
                .external_type   = GL_FLOAT,
        });
    }

    void pre_filter_env_map(GLuint env_map)
    {
        SPDLOG_INFO("pre filter env map...");

        auto                   capture_proj  = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
        std::vector<glm::mat4> capture_views = {glm::lookAt(glm::vec3(0, 0, 0), POSITIVE_X, NEGATIVE_Y),
                                                glm::lookAt(glm::vec3(0, 0, 0), NEGATIVE_X, NEGATIVE_Y),
                                                glm::lookAt(glm::vec3(0, 0, 0), POSITIVE_Y, POSITIVE_Z),
                                                glm::lookAt(glm::vec3(0, 0, 0), NEGATIVE_Y, NEGATIVE_Z),
                                                glm::lookAt(glm::vec3(0, 0, 0), POSITIVE_Z, NEGATIVE_Y),
                                                glm::lookAt(glm::vec3(0, 0, 0), NEGATIVE_Z, NEGATIVE_Y)};

        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        GLsizei mip_size = CUBE_SIZE;
        // per level of mipmap
        for (GLint level = 0; level < TOTAL_CUBE_MIP_LEVELS; ++level, mip_size /= 2)
        {

            glViewport(0, 0, mip_size, mip_size);
            glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_size, mip_size);

            float roughness = (float) level / (float) (TOTAL_CUBE_MIP_LEVELS - 1);

            // per face of cubemap
            for (GLuint i = 0; i < 6; ++i)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                       filtered_env_map, level);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                shader_envmap.update_per_frame(capture_views[i], capture_proj);
                shader_envmap.draw(model_cube, env_map, roughness);
            }
        }
    }

    void intgrate_brdf()
    {
        SPDLOG_INFO("integrate BRDF...");

        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, LUT_SIZE, LUT_SIZE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_lut, 0);

        glViewport(0, 0, LUT_SIZE, LUT_SIZE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader_int_brdf.draw(model_square);
    }
};

class TestEngine : public Engine
{
    GLuint cube_map = load_cube_map({
            .pos_x = TEX_SKY + "sky_Right.png",
            .neg_x = TEX_SKY + "sky_Left.png",
            .pos_y = TEX_SKY + "sky_Up.png",
            .neg_y = TEX_SKY + "sky_Down.png",
            .pos_z = TEX_SKY + "sky_Back.png",
            .neg_z = TEX_SKY + "sky_Front.png",
    });

    SplitSumApproximate split_sum;

    Model              model_square        = Model::load_obj(MODEL_SQUARE)[0];
    Model              model_cube          = Model::load_obj(MODEL_CUBE)[0];
    Model              model_spere         = Model::load_obj(MODEL_SPHERE)[0];
    std::vector<Model> model_sphere_matrix = Model::load_obj(MODEL_SPHERE_MATRIX);

    float     roughness = 0.2f;
    glm::vec3 F0        = glm::vec3(0.7, 0.7, 0.6);

    ShaderEnvMap  shader_envmap;
    ShaderDiffuse shader_lambert;
    ShaderIBL     shader_ibl;

    void init() override
    {
        // gl setting
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
        glDepthFunc(GL_LEQUAL);

        // linearly interpolate across cube face
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        // filter environment map
        split_sum.pre_filter_env_map(cube_map);

        // integrate BRDF
        split_sum.intgrate_brdf();

        model_square.tex_diffuse.has = true;
        model_square.tex_diffuse.id  = split_sum.brdf_lut;

        shader_ibl.init((float) split_sum.TOTAL_CUBE_MIP_LEVELS);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport_(0, 0, window.width, window.height);
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw filtered env map as skybox
        shader_envmap.update_per_frame(camera.view_matrix(), Camera::proj_matrix(), split_sum.TOTAL_CUBE_MIP_LEVELS);
        shader_envmap.draw(model_cube, split_sum.filtered_env_map, roughness);

        // draw brdf lut on square
        shader_lambert.set_uniform({
                {shader_lambert.m_view, {._mat4 = camera.view_matrix()}},
                {shader_lambert.m_proj, {._mat4 = Camera::proj_matrix()}},
        });
        shader_lambert.draw(model_square);

        // draw obj using ibl
        //        shader_ibl.udpate_per_frame(camera.view_matrix(), Camera::proj_matrix(), camera.get_pos());
        //        shader_ibl.draw(model_spere, split_sum.filtered_env_map, split_sum.brdf_lut, roughness, F0);
        //        for (auto & m : model_sphere_matrix)
        //            shader_ibl.draw(m, split_sum.filtered_env_map, split_sum.brdf_lut, roughness, F0);
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);
        ImGui::SliderFloat("roughness", &roughness, 0.f, 1.f);
        ImGui::End();
    }
};

int main()
{
    auto engine = TestEngine();
    engine.engine_main();
    return 0;
}