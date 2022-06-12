#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "frame-config.hpp"
#include "core/shader.h"
#include "core/rt-object.h"


class ShaderTexVisual
{
public:
    Shader2 shader = {SHADER + "tex2d-visual/tex-visual.vert",
                      SHADER + "tex2d-visual/tex-visual.frag"};

    /**
     * 纹理可视化
     * @param model 应该是正方形
     * @param tex_image_ 要可视化的纹理
     * @param channel_ 0 表示 rgb 三种颜色，1234 分别表示某个通道
     */
    void draw(const RTObject &obj, GLuint tex_image, int channel = 0)
    {
        glBindTexture_(GL_TEXTURE_2D, 0, tex_image);
        shader.set_uniform({
                {"channel", channel},
                {"tex_image", 0},
        });
        obj.mesh.draw();
    }
};