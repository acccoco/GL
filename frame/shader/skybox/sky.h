#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "frame-config.hpp"
#include "core/shader.h"


class ShaderSky
{
public:
    Shader2 shader = {SHADER + "skybox/sky.vert", SHADER + "skybox/sky.frag"};

    void init(const glm::mat4 &proj)
    {
        shader.set_uniform({
                {"m_proj", proj},
        });
    }

    void udpate_per_frame(const glm::mat4 &view, GLuint cube_map)
    {
        glBindTexture_(GL_TEXTURE_CUBE_MAP, 0, cube_map);
        shader.set_uniform({
                {"m_view", view},
                {"tex_cube", 0},
        });
    }

    static void draw(const RTObject &obj) { obj.mesh.draw(); }
};