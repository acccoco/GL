#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.hpp"
#include "core/shader.h"
#include "core/model.h"


class ShaderBlinnPhong : public Shader
{
protected:
    UniformAttributeM4fv m_view{"m_view", this};
    UniformAttributeM4fv m_proj{"m_proj", this};
    UniformAttributeM4fv m_model{"m_model", this};
    UniformAttribute1i has_diffuse{"has_diffuse", this};
    UniformAttribute1i tex_diffuse{"tex_diffuse", this};
    UniformAttribute3fv camera_pos{"camera_pos", this};
    UniformAttribute3fv ks{"ks", this};
    UniformAttribute3fv kd{"kd", this};
    UniformAttribute3fv light_pos{"light_pos", this};
    UniformAttribute1f light_indensity{"light_indensity", this};

public:
    ShaderBlinnPhong()
        : Shader(SHADER + "blinn-phong/blinn-phong.vert", SHADER + "blinn-phong/blinn-phong.frag")
    {
        uniform_attrs_location_init();
    }

    void init(const glm::mat4 &proj)
    {
        glUseProgram(program_id);
        m_proj.set(proj);
    }

    void update_per_frame(const glm::mat4 &view, const glm::vec3 &cam_pos, const glm::vec3 &light_pos_, float light_ind)
    {
        glUseProgram(program_id);
        m_view.set(view);
        camera_pos.set(cam_pos);
        light_pos.set(light_pos_);
        light_indensity.set(light_ind);
    }

    void draw(const Model &model)
    {
        glUseProgram(program_id);

        m_model.set(model.model_matrix());

        kd.set(model.color_diffuse);
        ks.set(model.color_specular);
        has_diffuse.set(model.tex_diffuse.has);
        if (model.tex_diffuse.has) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model.tex_diffuse.id);
            tex_diffuse.set(0);
        }

        model.mesh.draw();
    }
};