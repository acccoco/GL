#pragma once

#include <cstdlib>

#include "core/shader.h"

class ShaderDepth : public Shader
{
    UniformAttribute m_view{"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj{"m_proj", this, UniAttrType::MAT4};
    UniformAttribute m_model{"m_model", this, UniAttrType::MAT4};

public:
    ShaderDepth()
        : Shader(EXAMPLES + "pcss/depth.vert", EXAMPLES + "pcss/depth.frag")
    {
        this->uniform_attrs_location_init();
    }

    void update_per_frame(const glm::mat4 &view, const glm::mat4 &proj)
    {
        this->set_uniform({
                {m_view, {._mat4 = view}},
                {m_proj, {._mat4 = proj}},
        });
    }

    void draw(const Model &model)
    {
        this->set_uniform({
                {m_model, {._mat4 = model.model_matrix()}},
        });
        model.mesh.draw();
    }
};


class ShaderPcss : public Shader
{
    UniformAttribute m_model = {"m_model", this, UniAttrType::MAT4};
    UniformAttribute m_view  = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj  = {"m_proj", this, UniAttrType::MAT4};

    UniformAttribute shadow_map = {"shadow_map", this, UniAttrType::INT};
    UniformAttribute light_vp   = {"light_vp", this, UniAttrType::MAT4};
    UniformAttribute light_pos  = {"light_pos", this, UniAttrType::VEC3};
    UniformAttribute camera_pos = {"camera_pos", this, UniAttrType::VEC3};

    UniformAttribute kd          = {"kd", this, UniAttrType::VEC3};
    UniformAttribute ks          = {"ks", this, UniAttrType::VEC3};
    UniformAttribute has_diffuse = {"has_diffuse", this, UniAttrType::INT};
    UniformAttribute tex_diffuse = {"tex_diffuse", this, UniAttrType::INT};

    UniformAttribute rand_seed = {"rand_seed", this, UniAttrType::VEC3};

public:
    ShaderPcss()
        : Shader(EXAMPLES + "pcss/pcss.vert", EXAMPLES + "pcss/pcss.frag")
    {
        this->uniform_attrs_location_init();
    }

    void udpate_per_frame(const glm::mat4 &camera_view, const glm::mat4 &camera_proj, const glm::mat4 &light_vp_,
                          const glm::vec3 &light_pos_, const glm::vec3 &camera_pos_)
    {
        this->set_uniform({
                {m_view, {._mat4 = camera_view}},
                {m_proj, {._mat4 = camera_proj}},
                {light_vp, {._mat4 = light_vp_}},
                {light_pos, {._vec3 = light_pos_}},
                {camera_pos, {._vec3 = camera_pos_}},
                {rand_seed, {._vec3 = glm::vec3(std::rand(), std::rand(), 0)}},
        });
    }

    void draw(const Model &model, GLuint shadow_map_)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadow_map_);

        if (model.tex_diffuse.has)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, model.tex_diffuse.id);
        }

        this->set_uniform({

                {kd, {._vec3 = model.color_diffuse}},
                {ks, {._vec3 = model.color_specular}},
                {has_diffuse, {._int = model.tex_diffuse.has}},
                {tex_diffuse, {._int = 1}},
                {shadow_map, {._int = 0}},
                {m_model, {._mat4 = model.model_matrix()}},
        });
        model.mesh.draw();
    }
};
