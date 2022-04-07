#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/misc.h"
#include "core/model.h"
#include "core/texture.h"

#include "shader/sky/sky.h"
#include "shader/lambert/lambert.h"


struct DepthFramebuffer {
    GLuint frame_buffer{};
    GLuint depth_buffer;
    GLuint tex_depth;
    const GLsizei size = 1024;

    DepthFramebuffer()
    {
        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

        depth_buffer = create_depth_buffer(size, size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

        tex_depth = create_tex(size, size);
    }
};


class TestEngine : public Engine
{
    DepthFramebuffer buffer;

    std::vector<Model> model_three_obj = Model::load_obj(MODEL_THREE_OBJS);
    std::vector<Model> model_matrix = Model::load_obj(MODEL_SPHERE_MATRIX);
    std::vector<Model> model_202 = Model::load_obj(MODEL_202_CHAN);
    std::vector<Model> model_diona = Model::load_obj(MODEL_DIONA);
    Model model_cube = Model::load_obj(MODEL_CUBE)[0];
    Model model_light = Model::load_obj(MODEL_LIGHT)[0];
    Model model_floor = Model::load_obj(MODLE_FLOOR)[0];

    void tick_pre_render() override {}

    void tick_render() override {}

    void tick_gui() override {}
};


int main()
{
    auto engine = TestEngine();
    engine.engine_main();
}
