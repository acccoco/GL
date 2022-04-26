#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.hpp"
#include "core/shader.h"
#include "core/model.h"

class ShaderLambert : public Shader
{
    UniformAttributeM4fv m_view{"m_view"};
    UniformAttributeM4fv m_proj{"m_proj"};
    UniformAttributeM4fv m_model{"m_model"};
    UniformAttribute1i has_diffuse{"has_diffuse"};
    UniformAttribute1i tex_diffuse{"tex_diffuse"};
    UniformAttribute3fv kd{"kd"};

public:
    ShaderLambert()
        : Shader(SHADER + "lambert/lambert.vert", SHADER + "lambert/lambert.frag")
    {
        for (auto u_attr: std::vector<UniformAttribute *>{&m_view, &m_proj, &m_model, &has_diffuse, &tex_diffuse, &kd})
            u_attr->init_location(program_id);
    }

    void init(const glm::mat4 &proj)
    {
        glUseProgram(program_id);
        m_proj.set(proj);
    }

    void update_per_fame(const glm::mat4 &view)
    {
        glUseProgram(program_id);
        m_view.set(view);
    }

    void udpate_per_frame(const glm::mat4 &view, const glm::mat4 &proj)
    {
        m_proj.set(proj);
        m_view.set(view);
    }

    void draw(const Model &model)
    {
        glUseProgram(program_id);
        m_model.set(model.model_matrix());
        kd.set(model.color_diffuse);
        has_diffuse.set(model.tex_diffuse.has);
        if (model.tex_diffuse.has) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model.tex_diffuse.id);
            tex_diffuse.set(0);
        }

        model.mesh.draw();
    }
};