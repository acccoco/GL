#pragma once

#include "core/shader.h"


class ShaderPrefilterEnvMap : public Shader
{
protected:
    UniformAttributeM4fv m_view{"m_view"};
    UniformAttributeM4fv m_proj{"m_proj"};
    UniformAttribute1i env_map{"env_map"};
    UniformAttribute1f roughness{"roughness"};

public:
    ShaderPrefilterEnvMap()
        : Shader(EXAMPLES + "split-sum-approximate/cube-map-common.vert",
                 EXAMPLES + "split-sum-approximate/prefilter-env-map.frag")
    {
        for (auto u_attr: std::vector<UniformAttribute *>{&m_view, &m_proj, &env_map, &roughness})
            u_attr->init_location(program_id);
    }

    void update_per_frame(const glm::mat4 &view, const glm::mat4 &proj)
    {
        m_view.set(view);
        m_proj.set(proj);
    }

    void draw(const Model &model, GLuint cube_map, float roughness_)
    {
        glUseProgram(program_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);
        env_map.set(0);
        roughness.set(roughness_);

        model.mesh.draw();
    }
};

class ShaderEnvMap : public Shader
{
protected:
    UniformAttributeM4fv m_view{"m_view"};
    UniformAttributeM4fv m_proj{"m_proj"};
    UniformAttribute1i tex_cube{"tex_cube"};
    UniformAttribute1f roughness{"roughness"};
    UniformAttribute1i total_mip_level{"total_mip_level"};

public:
    ShaderEnvMap()
        : Shader(EXAMPLES + "split-sum-approximate/cube-map-common.vert",
                 EXAMPLES + "split-sum-approximate/env-map.frag")
    {
        std::vector<UniformAttribute *> uniforms = {&m_view, &m_proj, &tex_cube, &roughness, &total_mip_level};
        for (auto uniform: uniforms)
            uniform->init_location(program_id);
    }

    void update_per_frame(const glm::mat4 &view, const glm::mat4 &proj, int total_levels)
    {
        total_mip_level.set(total_levels);
        m_view.set(view);
        m_proj.set(proj);
    }

    void draw(const Model &model, GLuint cube_map, float roughness_)
    {
        glUseProgram(program_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);
        tex_cube.set(0);
        roughness.set(roughness_);

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
    UniformAttributeM4fv m_model{"m_model"};
    UniformAttributeM4fv m_view{"m_view"};
    UniformAttributeM4fv m_proj{"m_proj"};
    UniformAttribute1i filtered_env_map{"filtered_env_map"};
    UniformAttribute1i brdf_lut{"brdf_lut"};
    UniformAttribute3fv camera_pos{"camera_pos"};
    UniformAttribute1f total_cube_mip_level{"total_cube_mip_level"};
    UniformAttribute1f roughness{"roughness"};
    UniformAttribute3fv F0{"F0"};

    ShaderIBL()
        : Shader(EXAMPLES + "split-sum-approximate/common.vert", EXAMPLES + "split-sum-approximate/ibl.frag")
    {
        std::vector<UniformAttribute *> uniforms = {
                &m_model,   &m_view, &m_proj, &filtered_env_map, &brdf_lut, &camera_pos, &total_cube_mip_level,
                &roughness, &F0};
        for (auto uniform: uniforms)
            uniform->init_location(program_id);
    }

    void init(float total_mip_level)
    {
        total_cube_mip_level.set(total_mip_level);
    }

    void udpate_per_frame(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec3 &camera_pos_)
    {
        m_view.set(view);
        m_proj.set(proj);
        camera_pos.set(camera_pos_);
    }

    void draw(const Model &model, GLuint fitered_env_map_, GLuint brdf_lut_, float roughness_, const glm::vec3 &F0_)
    {
        glUseProgram(program_id);
        m_model.set(model.model_matrix());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, fitered_env_map_);
        filtered_env_map.set(0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brdf_lut_);
        brdf_lut.set(1);

        roughness.set(roughness_);
        F0.set(F0_);

        model.mesh.draw();
    }
};