#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.hpp"
#include "core/shader.h"
#include "core/model.h"

class ShaderDiffuse : public Shader
{
public:
    UniformAttribute m_view{"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj{"m_proj", this, UniAttrType::MAT4};
    UniformAttribute m_model{"m_model", this, UniAttrType::MAT4};
    UniformAttribute has_diffuse{"has_diffuse", this, UniAttrType::INT};
    UniformAttribute tex_diffuse{"tex_diffuse", this, UniAttrType::INT};
    UniformAttribute kd{"kd", this, UniAttrType::VEC3};


    ShaderDiffuse()
        : Shader(SHADER + "diffuse/diffuse.vert", SHADER + "diffuse/diffuse.frag")
    {
        uniform_attrs_location_init();
    }

    void init(const glm::mat4 &proj)
    {
        this->set_uniform({
                {m_proj, {._mat4 = proj}},
        });
    }

    void update_per_fame(const glm::mat4 &view)
    {
        this->set_uniform({
                {m_view, {._mat4 = view}},
        });
    }

    void draw(const Model &model)
    {
        if (model.tex_diffuse.has)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model.tex_diffuse.id);
        }
        this->set_uniform({
                {m_model, {._mat4 = model.model_matrix()}},
                {kd, {._vec3 = model.color_diffuse}},
                {has_diffuse, {._int = model.tex_diffuse.has}},
                {tex_diffuse, {._int = 0}},
        });
        model.mesh.draw();
    }
};