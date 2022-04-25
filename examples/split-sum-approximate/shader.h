#pragma once

#include "core/shader.h"


class ShaderEnvmap : public Shader
{
protected:
    UniformAttributeM4fv m_view{"m_view"};
    UniformAttributeM4fv m_proj{"m_proj"};
    UniformAttribute1i env_map{"env_map"};
    UniformAttribute1f roughness{"roughness"};

public:
    ShaderEnvmap()
        : Shader(EXAMPLES + "split-sum-approximate/envmap.vert", EXAMPLES + "split-sum-approximate/envmap.frag")
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