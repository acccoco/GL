#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/model.h"
#include "core/misc.h"
#include "function/skybox/skybox.h"

#include "./shader.h"

struct CubeFramebuffer {
    GLuint frame_buffer{};
    GLuint depth_buffer;
    GLuint tex_cube;
    const GLsizei size = 1024;

    CubeFramebuffer()
    {
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        depth_buffer = create_depth_buffer(size, size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

        tex_cube = create_cube_map(size);
    }
};

class TestEngine : public Engine
{
    GLuint cube_map = load_cube_map(TEX_SKY + "sky_Right.png", TEX_SKY + "sky_Left.png", TEX_SKY + "sky_Up.png",
                                    TEX_SKY + "sky_Down.png", TEX_SKY + "sky_Back.png", TEX_SKY + "sky_Front.png");

    CubeFramebuffer buffer;
    Model model_cube = Model::load_obj(MODEL_CUBE)[0];
    ShaderEnvmap shader_envmap;
    const float roughness = 0.2f;

    CubeMapVisual cubemap_visual;

    void init() override
    {
        // gl setting
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
        glDepthFunc(GL_LEQUAL);

        // filter environment map
        SPDLOG_INFO("pre filter env map...");
        glBindFramebuffer(GL_FRAMEBUFFER, buffer.frame_buffer);
        glViewport(0, 0, buffer.size, buffer.size);
        glm::mat4 m_proj = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 20.f);
        auto draw_dir = [&](GLenum textarget, const glm::vec3 &front, const glm::vec3 &up) {
            // textarget: for cube map, specify which face is to be attached
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textarget, buffer.tex_cube, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 m_view = glm::lookAt(glm::vec3(0, 0, 0), front, up);
            shader_envmap.update_per_frame(m_view, m_proj);
            shader_envmap.draw(model_cube, cube_map, roughness);
        };
        // for historical reason of cubemap, reverse UP direction of camera.
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_X, POSITIVE_X, -POSITIVE_Y);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, NEGATIVE_X, -POSITIVE_Y);
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, POSITIVE_Z, -POSITIVE_Y);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, NEGATIVE_Z, -POSITIVE_Y);// front
        draw_dir(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, POSITIVE_Y, POSITIVE_Z);
        draw_dir(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, NEGATIVE_Y, NEGATIVE_Z);// down
        SPDLOG_INFO("pre filter env map, done");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport_(0, 0, window.width, window.height);
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cubemap_visual.draw_as_skybox(camera.view_matrix(), Camera::proj_matrix(), buffer.tex_cube);
    }
};

int main()
{
    auto engine = TestEngine();
    engine.engine_main();
    return 0;
}