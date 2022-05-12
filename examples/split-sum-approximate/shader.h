#pragma once

#include "core/shader.h"


class ShaderPrefilterEnvMap : public Shader
{
protected:
    UniformAttribute m_view    = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj    = {"m_proj", this, UniAttrType::MAT4};
    UniformAttribute env_map   = {"env_map", this, UniAttrType::INT};
    UniformAttribute roughness = {"roughness", this, UniAttrType::FLOAT};

public:
    ShaderPrefilterEnvMap()
        : Shader(EXAMPLES + "split-sum-approximate/cube-map-common.vert",
                 EXAMPLES + "split-sum-approximate/prefilter-env-map.frag")
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

    void draw(const Model &model, GLuint cube_map, float roughness_)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);
        this->set_uniform({
                {env_map, {._int = 0}},
                {roughness, {._float = roughness_}},
        });
        model.mesh.draw();
    }
};

class ShaderEnvMap : public Shader
{
protected:
    UniformAttribute m_view          = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj          = {"m_proj", this, UniAttrType::MAT4};
    UniformAttribute tex_cube        = {"tex_cube", this, UniAttrType::INT};
    UniformAttribute roughness       = {"roughness", this, UniAttrType::FLOAT};
    UniformAttribute total_mip_level = {"total_mip_level", this, UniAttrType::INT};

public:
    ShaderEnvMap()
        : Shader(EXAMPLES + "split-sum-approximate/cube-map-common.vert",
                 EXAMPLES + "split-sum-approximate/env-map.frag")
    {
        this->uniform_attrs_location_init();
    }

    void update_per_frame(const glm::mat4 &view, const glm::mat4 &proj, int total_levels)
    {
        this->set_uniform({
                {total_mip_level, {._int = total_levels}},
                {m_view, {._mat4 = view}},
                {m_proj, {._mat4 = proj}},
        });
    }

    void draw(const Model &model, GLuint cube_map, float roughness_)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);
        this->set_uniform({
                {tex_cube, {._int = 0}},
                {roughness, {._float = roughness_}},
        });

        model.mesh.draw();
    }
};

class ShaderIntBRDF : public Shader
{
public:
    ShaderIntBRDF()
        : Shader(EXAMPLES + "split-sum-approximate/square-common.vert",
                 EXAMPLES + "split-sum-approximate/int-brdf.frag")
    {}

    void draw(const Model &model)
    {
        glUseProgram(program_id);
        model.mesh.draw();
    }
};

class ShaderIBL : public Shader
{
public:
    UniformAttribute m_model              = {"m_model", this, UniAttrType::MAT4};
    UniformAttribute m_view               = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj               = {"m_proj", this, UniAttrType::MAT4};
    UniformAttribute filtered_env_map     = {"filtered_env_map", this, UniAttrType::INT};
    UniformAttribute brdf_lut             = {"brdf_lut", this, UniAttrType::INT};
    UniformAttribute camera_pos           = {"camera_pos", this, UniAttrType::VEC3};
    UniformAttribute total_cube_mip_level = {"total_cube_mip_level", this, UniAttrType::FLOAT};
    UniformAttribute roughness            = {"roughness", this, UniAttrType::FLOAT};
    UniformAttribute F0                   = {"F0", this, UniAttrType::VEC3};

    ShaderIBL()
        : Shader(EXAMPLES + "split-sum-approximate/common.vert", EXAMPLES + "split-sum-approximate/ibl.frag")
    {
        this->uniform_attrs_location_init();
    }

    void init(float total_mip_level)
    {
        this->set_uniform({
                {total_cube_mip_level, {._float = total_mip_level}},
        });
    }

    void udpate_per_frame(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec3 &camera_pos_)
    {
        this->set_uniform({
                {m_view, {._mat4 = view}},
                {m_proj, {._mat4 = proj}},
                {camera_pos, {._vec3 = camera_pos_}},
        });
    }

    void draw(const Model &model, GLuint fitered_env_map_, GLuint brdf_lut_, float roughness_, const glm::vec3 &F0_)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, fitered_env_map_);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brdf_lut_);

        this->set_uniform({
                {m_model, {._mat4 = model.model_matrix()}},
                {filtered_env_map, {._int = 0}},
                {brdf_lut, {._int = 1}},
                {roughness, {._float = roughness_}},
                {F0, {._vec3 = F0_}},
        });

        model.mesh.draw();
    }
};