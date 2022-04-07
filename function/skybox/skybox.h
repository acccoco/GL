#pragma once

#include "config.hpp"
#include "core/model.h"
#include "function/skybox//shader/sky.h"


class SkyBox
{
    ShaderSky shader_sky;
    Model model_cube = Model::load_obj(MODEL_CUBE)[0];
    GLuint default_sky_cubemap =
            load_cube_map(TEX_SKY + "sky_Right.png", TEX_SKY + "sky_Left.png", TEX_SKY + "sky_Up.png",
                          TEX_SKY + "sky_Down.png", TEX_SKY + "sky_Back.png", TEX_SKY + "sky_Front.png");

public:
    // after exec, DEPTH_TEST will be enabled
    void draw_default_sky(const glm::mat4 &view, const glm::mat4 &proj)
    {
        glDisable(GL_DEPTH_TEST);
        shader_sky.init(proj);
        shader_sky.udpate_per_frame(view, default_sky_cubemap);
        shader_sky.draw(model_cube);
        glEnable(GL_DEPTH_TEST);
    }
};


class CubeMapVisual
{
    ShaderSky shader_sky;
    Model model_cube = Model::load_obj(MODEL_CUBE)[0];

public:
    // after exec, DEPTH_TEST will be enabled
    void draw_as_skybox(const glm::mat4 &view, const glm::mat4 &proj, GLuint tex_cube)
    {
        glDisable(GL_DEPTH_TEST);
        shader_sky.init(proj);
        shader_sky.udpate_per_frame(view, tex_cube);
        shader_sky.draw(model_cube);
        glEnable(GL_DEPTH_TEST);
    }
};