#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "frame-config.hpp"
#include "core/shader.h"


class ShaderBlinnPhong
{
public:
    Shader2 shader = {SHADER + "blinn-phong/blinn-phong.vert",
                      SHADER + "blinn-phong/blinn-phong.frag"};

    void init(const glm::mat4 &proj)
    {
        shader.set_uniform({
                {"m_proj", proj},
        });
    }

    void update_per_frame(const glm::mat4 &view, const glm::vec3 &cam_pos,
                          const glm::vec3 &light_pos_, float light_ind)
    {
        shader.set_uniform({
                {"m_view", view},
                {"camera_pos", cam_pos},
                {"light_pos", light_pos_},
                {"light_indensity", light_ind},
        });
    }

    void draw(const RTObject &obj)
    {
        const Material &mat = obj.mesh.mat;
        if (mat.has_tex_basecolor())
            glBindTexture_(GL_TEXTURE_2D, 0, mat.metallic_roughness.tex_base_color);
        shader.set_uniform({
                {"m_model", obj.matrix()},
                {"kd", glm::vec3(mat.metallic_roughness.base_color)},
                {"ks", glm::vec3(0.6f)},
                {"has_diffuse", mat.has_tex_basecolor()},
                {"tex_diffuse", 0},
        });
        obj.mesh.draw();
    }
};