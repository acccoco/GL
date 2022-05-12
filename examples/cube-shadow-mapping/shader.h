#pragma once

#include "config.hpp"
#include "core/model.h"
#include "core/shader.h"


class ShaderDepth : public Shader
{
    UniformAttribute m_view{"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj{"m_proj", this, UniAttrType::MAT4};
    UniformAttribute m_model{"m_model", this, UniAttrType::MAT4};

public:
    ShaderDepth()
        : Shader(EXAMPLES + "cube-shadow-mapping/distance-to-light.vert",
                 EXAMPLES + "cube-shadow-mapping/distance-to-light.frag")
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


class ShaderShadowMapping : public Shader
{
    UniformAttribute m_view  = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj  = {"m_proj", this, UniAttrType::MAT4};
    UniformAttribute m_model = {"m_model", this, UniAttrType::MAT4};

    UniformAttribute light_pos  = {"light_pos", this, UniAttrType::VEC3};
    UniformAttribute camera_pos = {"camera_pos", this, UniAttrType::VEC3};

    UniformAttribute shadow_map_cube = {"shadow_map_cube", this, UniAttrType::INT};

    UniformAttribute has_diffuse = {"has_diffuse", this, UniAttrType::INT};
    UniformAttribute tex_diffuse = {"tex_diffuse", this, UniAttrType::INT};
    UniformAttribute kd          = {"kd", this, UniAttrType::VEC3};
    UniformAttribute ks          = {"ks", this, UniAttrType::VEC3};

public:
    ShaderShadowMapping()
        : Shader(EXAMPLES + "cube-shadow-mapping/shadow-mapping.vert",
                 EXAMPLES + "cube-shadow-mapping/shadow-mapping.frag")
    {
        this->uniform_attrs_location_init();
    }

    void init(const glm::mat4 &proj)
    {
        this->set_uniform({
                {m_proj, {._mat4 = proj}},
        });
    }

    void update_per_frame(const glm::mat4 &view, const glm::vec3 &cam_pos, const glm::vec3 &light_pos_)
    {
        this->set_uniform({
                {m_view, {._mat4 = view}},
                {camera_pos, {._vec3 = cam_pos}},
                {light_pos, {._vec3 = light_pos_}},
        });
    }

    void draw(const Model &m, GLuint shadow_map_cube_)
    {
        glUseProgram(program_id);
        if (m.tex_diffuse.has)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m.tex_diffuse.id);
        }
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_map_cube_);

        this->set_uniform({
                {m_model, {._mat4 = m.model_matrix()}},
                {kd, {._vec3 = m.color_diffuse}},
                {ks, {._vec3 = m.color_specular}},
                {has_diffuse, {._int = m.tex_diffuse.has}},
                {tex_diffuse, {._int = 0}},
                {shadow_map_cube, {._int = 1}},
        });

        m.mesh.draw();
    }
};