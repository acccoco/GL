#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.hpp"
#include "core/shader.h"
#include "core/model.h"

class ShaderTexVisual : public Shader
{
public:
    UniformAttribute tex_image = {"tex_image", this, UniAttrType::INT};

    /// 0 表示 rgb 三种颜色，1234 分别表示某个通道
    UniformAttribute channel = {"channel", this, UniAttrType::INT};


    ShaderTexVisual()
        : Shader(SHADER + "tex2d-visual/tex-visual.vert", SHADER + "tex2d-visual/tex-visual.frag")
    {
        this->uniform_attrs_location_init();
    }

    void draw(const Model &model, GLuint tex_image_)
    {
        glUseProgram(program_id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_image_);
        this->set_uniform({
                {tex_image, {._int = 0}},
        });

        model.mesh.draw();
    }
};