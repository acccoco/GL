#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "frame-config.hpp"
#include "core/shader.h"
#include "core/model.h"


class ShaderBlinnPhong : public Shader
{
public:
    UniformAttribute m_view{"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj{"m_proj", this, UniAttrType::MAT4};
    UniformAttribute m_model{"m_model", this, UniAttrType::MAT4};
    UniformAttribute has_diffuse{"has_diffuse", this, UniAttrType::INT};
    UniformAttribute tex_diffuse{"tex_diffuse", this, UniAttrType::INT};
    UniformAttribute camera_pos{"camera_pos", this, UniAttrType::VEC3};
    UniformAttribute ks{"ks", this, UniAttrType::VEC3};
    UniformAttribute kd{"kd", this, UniAttrType::VEC3};
    UniformAttribute light_pos{"light_pos", this, UniAttrType::VEC3};
    UniformAttribute light_indensity{"light_indensity", this, UniAttrType::FLOAT};


    ShaderBlinnPhong()
        : Shader(SHADER + "blinn-phong/blinn-phong.vert", SHADER + "blinn-phong/blinn-phong.frag")
    {
        uniform_attrs_location_init();
    }

    void init(const glm::mat4 &proj)
    {
        glUseProgram(program_id);
        this->set_uniform({
                {m_proj, {._mat4 = proj}},
        });
    }

    void update_per_frame(const glm::mat4 &view, const glm::vec3 &cam_pos, const glm::vec3 &light_pos_, float light_ind)
    {
        glUseProgram(program_id);
        this->set_uniform({
                {m_view, {._mat4 = view}},
                {camera_pos, {._vec3 = cam_pos}},
                {light_pos, {._vec3 = light_pos_}},
                {light_indensity, {._float = light_ind}},
        });
    }

    void draw(const Model &model)
    {
        glUseProgram(program_id);

        if (model.tex_diffuse.has)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model.tex_diffuse.id);
        }

        this->set_uniform({
                {m_model, {._mat4 = model.model_matrix()}},
                {kd, {._vec3 = model.color_diffuse}},
                {ks, {._vec3 = model.color_specular}},
                {has_diffuse, {._int = model.tex_diffuse.has}},
                {tex_diffuse, {._int = 0}},
        });

        model.mesh.draw();
    }
};