#pragma once

#include <cstdlib>

#include "core/shader.h"

class ShaderDepth : public Shader
{
    UniformAttributeM4fv m_view{"m_view"};
    UniformAttributeM4fv m_proj{"m_proj"};
    UniformAttributeM4fv m_model{"m_model"};

public:
    ShaderDepth()
        : Shader(EXAMPLES + "pcss/depth.vert", EXAMPLES + "pcss/depth.frag")
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


class ShaderPcss : public Shader
{
    UniformAttributeM4fv m_model{"m_model"};
    UniformAttributeM4fv m_view{"m_view"};
    UniformAttributeM4fv m_proj{"m_proj"};

    UniformAttribute1i shadow_map{"shadow_map"};
    UniformAttributeM4fv light_vp{"light_vp"};
    UniformAttribute3fv light_pos{"light_pos"};
    UniformAttribute3fv camera_pos{"camera_pos"};

    UniformAttribute3fv kd{"kd"};
    UniformAttribute3fv ks{"ks"};
    UniformAttribute1i has_diffuse{"has_diffuse"};
    UniformAttribute1i tex_diffuse{"tex_diffuse"};

    UniformAttribute3fv rand_seed{"rand_seed"};

public:
    ShaderPcss()
        : Shader(EXAMPLES + "pcss/pcss.vert", EXAMPLES + "pcss/pcss.frag")
    {
        for (auto u_attr:
             std::vector<UniformAttribute *>{&m_view, &m_proj, &m_model, &shadow_map, &light_vp, &light_pos, &kd,
                                             &has_diffuse, &tex_diffuse, &camera_pos, &ks, &rand_seed})
            u_attr->init_location(program_id);
    }

    void udpate_per_frame(const glm::mat4 &camera_view, const glm::mat4 &camera_proj, const glm::mat4 &light_vp_,
                          const glm::vec3 &light_pos_, const glm::vec3 &camera_pos_)
    {
        m_view.set(camera_view);
        m_proj.set(camera_proj);
        light_vp.set(light_vp_);
        light_pos.set(light_pos_);
        camera_pos.set(camera_pos_);

        rand_seed.set(glm::vec3(std::rand(), std::rand(), 0));
    }

    void draw(const Model &model, GLuint shadow_map_)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadow_map_);
        shadow_map.set(0);

        kd.set(model.color_diffuse);
        ks.set(model.color_specular);
        has_diffuse.set(model.tex_diffuse.has);
        if (model.tex_diffuse.has) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, model.tex_diffuse.id);
            tex_diffuse.set(1);
        }
        m_model.set(model.model_matrix());
        model.mesh.draw();
    }
};
