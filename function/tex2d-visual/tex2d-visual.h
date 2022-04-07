#pragma once

#include "core/model.h"
#include "function/tex2d-visual/shader/tex-visual.h"


class Tex2DVisual
{
    Model model_square = Model::load_obj(MODEL_SQUARE)[0];
    ShaderTexVisual shader;

public:
    void show(GLuint tex_id)
    {
        shader.draw(model_square, tex_id);
    }
};