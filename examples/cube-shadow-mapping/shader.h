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
        : Shader(EXAMPLES + "cube-shadow-mapping/distance-to-light.vert",
                 EXAMPLES + "cube-shadow-mapping/distance-to-light.frag")
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


class ShaderShadowMapping : public Shader
{
    UniformAttributeM4fv m_view{"m_view"};
    UniformAttributeM4fv m_proj{"m_proj"};
    UniformAttributeM4fv m_model{"m_model"};
    UniformAttribute3fv light_pos{"light_pos"};
    UniformAttribute3fv camera_pos{"camera_pos"};
    UniformAttribute1i shadow_map_cube{"shadow_map_cube"};
    UniformAttribute1i has_diffuse{"has_diffuse"};
    UniformAttribute1i tex_diffuse{"tex_diffuse"};
    UniformAttribute3fv kd{"kd"};
    UniformAttribute3fv ks{"ks"};

public:
    ShaderShadowMapping()
        : Shader(EXAMPLES + "cube-shadow-mapping/shadow-mapping.vert",
                 EXAMPLES + "cube-shadow-mapping/shadow-mapping.frag")
    {
        std::vector<UniformAttribute *> uniforms = {&m_view, &m_proj,    &m_model,    &has_diffuse,     &tex_diffuse,
                                                    &kd,     &light_pos, &camera_pos, &shadow_map_cube, &ks};
        for (auto u_attr: uniforms)
            u_attr->init_location(program_id);
    }

    void init(const glm::mat4 &proj)
    {
        m_proj.set(proj);
    }

    void update_per_frame(const glm::mat4 &view, const glm::vec3 &cam_pos, const glm::vec3 &light_pos_)
    {
        m_view.set(view);
        camera_pos.set(cam_pos);
        light_pos.set(light_pos_);
    }

    void draw(const Model &m, GLuint shadow_map_cube_)
    {
        glUseProgram(program_id);
        m_model.set(m.model_matrix());

        kd.set(m.color_diffuse);
        ks.set(m.color_specular);
        has_diffuse.set(m.tex_diffuse.has);
        if (m.tex_diffuse.has) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m.tex_diffuse.id);
            tex_diffuse.set(0);
        }

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_map_cube_);
        shadow_map_cube.set(1);

        m.mesh.draw();
    }
};