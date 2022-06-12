#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "frame-config.hpp"
#include "core/shader.h"
#include "core/rt-object.h"

class ShaderDiffuse
{
public:
    Shader2 shader = {SHADER + "diffuse/diffuse.vert", SHADER + "diffuse/diffuse.frag"};

    void init(const glm::mat4 &proj) { shader.set_uniform({{"m_proj", proj}}); }

    void update_per_fame(const glm::mat4 &view) { shader.set_uniform({{"m_view", view}}); }

    void draw(const RTObject &obj)
    {
        const Material &mat = obj.mesh.mat;
        if (mat.has_tex_basecolor())
            glBindTexture_(GL_TEXTURE_2D, 0, mat.metallic_roughness.tex_base_color);
        shader.set_uniform({
                {"m_model", obj.matrix()},
                {"kd", glm::vec3(mat.metallic_roughness.base_color)},
                {"has_diffuse", mat.has_tex_basecolor()},
                {"tex_diffuse", 0},
        });
        obj.mesh.draw();
    }
};
