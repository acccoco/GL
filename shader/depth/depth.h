#pragma once

#include "config.hpp"
#include "core/model.h"
#include "core/shader.h"


class ShaderDepth : public Shader
{
    UniformAttributeM4fv m_view{"m_view"};
    UniformAttributeM4fv m_proj{"m_proj"};
    UniformAttributeM4fv m_model{"m_model"};

public:
    ShaderDepth()
        : Shader(SHADER + "depth/depth.vert", SHADER + "depth/depth.frag")
    {
        for (auto u_attr: std::vector<UniformAttribute *>{&m_view, &m_proj, &m_model})
            u_attr->init_location(program_id);
    }

    void update_per_frame(const glm::mat4 &view, const glm::mat4 &proj)
    {
        m_view.set(view);
        m_proj.set(proj);
    }

    void draw(const Model &model)
    {
        glUseProgram(program_id);
        m_model.set(model.model_matrix());
        model.mesh.draw();
    }
};
