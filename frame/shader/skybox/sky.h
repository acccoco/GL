#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "frame-config.hpp"
#include "core/shader.h"
#include "core/model.h"


class ShaderSky : public Shader
{
    UniformAttribute m_view{"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj{"m_proj", this, UniAttrType::MAT4};
    UniformAttribute tex_cube{"tex_cube", this, UniAttrType::INT};

public:
    ShaderSky()
        : Shader(SHADER + "skybox/sky.vert", SHADER + "skybox/sky.frag")
    {
        this->uniform_attrs_location_init();
    }

    void init(const glm::mat4 &proj)
    {
        this->set_uniform({
                {m_proj, {._mat4 = proj}},
        });
    }

    void udpate_per_frame(const glm::mat4 &view, GLuint cube_map)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);
        this->set_uniform({
                {m_view, {._mat4 = view}},
                {tex_cube, {._int = 0}},
        });
    }

    void draw(const Model &model)
    {
        glUseProgram(program_id);
        model.mesh.draw();
    }
};