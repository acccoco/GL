#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.hpp"
#include "core/shader.h"
#include "core/model.h"


const std::string cur_shader = EXAMPLE_CUR_PATH + "shader/";

class ShaderPRT : public Shader
{
public:
    UniformAttribute m_model = {"m_model", this, UniAttrType::MAT4};
    UniformAttribute m_view  = {"m_view", this, UniAttrType::MAT4};
    UniformAttribute m_proj  = {"m_proj", this, UniAttrType::MAT4};

    UniformAttribute SH_light_R = {"SH_light_R", this, UniAttrType::MAT3};
    UniformAttribute SH_light_G = {"SH_light_G", this, UniAttrType::MAT3};
    UniformAttribute SH_light_B = {"SH_light_B", this, UniAttrType::MAT3};


    ShaderPRT()
        : Shader(cur_shader + "prt.vert",
                 cur_shader + "prt.frag")
    {
        uniform_attrs_location_init();
    }
};