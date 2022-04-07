#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.hpp"
#include "core/shader.h"
#include "core/model.h"

class ShaderTexVisual : public Shader
{
protected:
    UniformAttribute1i tex_image{"tex_image"};

public:
    ShaderTexVisual()
        : Shader(FUNCTION + "tex2d-visual/shader/tex-visual.vert", FUNCTION + "tex2d-visual/shader/tex-visual.frag")
    {
        for (auto u_attr: std::vector<UniformAttribute *>{&tex_image})
            u_attr->init_location(program_id);
    }

    void draw(const Model &model, GLuint tex_image_)
    {
        glUseProgram(program_id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_image_);
        tex_image.set(0);

        model.mesh.draw();
    }
};