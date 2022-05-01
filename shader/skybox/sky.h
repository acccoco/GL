#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.hpp"
#include "core/shader.h"
#include "core/model.h"


class ShaderSky : public Shader
{
    UniformAttributeM4fv m_view{"m_view"};
    UniformAttributeM4fv m_proj{"m_proj"};
    UniformAttribute1i tex_cube{"tex_cube"};

public:
    ShaderSky()
        : Shader(SHADER + "skybox/sky.vert", SHADER + "skybox/sky.frag")
    {
        for (auto u_attr: std::vector<UniformAttribute *>{&m_view, &m_proj, &tex_cube})
            u_attr->init_location(program_id);
    }

    void init(const glm::mat4 &proj)
    {
        m_proj.set(proj);
    }

    void udpate_per_frame(const glm::mat4 &view, GLuint cube_map)
    {
        m_view.set(view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);
        tex_cube.set(0);
    }

    void draw(const Model &model)
    {
        glUseProgram(program_id);
        model.mesh.draw();
    }
};