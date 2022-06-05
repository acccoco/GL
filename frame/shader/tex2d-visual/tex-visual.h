#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "frame-config.hpp"
#include "core/shader.h"
#include "core/model.h"

class ShaderTexVisual : public Shader
{
public:
    UniformAttribute tex_image = {"tex_image", this, UniAttrType::INT};

    /// 0 表示 rgb 三种颜色，1234 分别表示某个通道
    UniformAttribute channel = {"channel", this, UniAttrType::INT};


    ShaderTexVisual()
        : Shader(SHADER + "tex2d-visual/tex-visual.vert", SHADER + "tex2d-visual/tex-visual.frag")
    {
        this->uniform_attrs_location_init();
    }

    /**
     * 纹理可视化
     * @param model 应该是正方形
     * @param tex_image_ 要可视化的纹理
     */
    void draw(const Model &model, GLuint tex_image_)
    {
        glUseProgram(program_id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_image_);
        this->set_uniform({
                {tex_image, {._int = 0}},
        });

        model.mesh.draw();
    }

    /**
     * 纹理可视化
     * @param model 应该是正方形
     * @param tex_image_ 要可视化的纹理
     * @param channel_ 显示哪些通道
     */
    void draw(const Model &model, GLuint tex_image_, int channel_)
    {
        this->set_uniform({{this->channel, {._int = channel_}}});
        draw(model, tex_image_);
    }
};