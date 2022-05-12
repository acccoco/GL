#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.hpp"
#include "core/shader.h"
#include "core/model.h"


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
        : Shader(EXAMPLES + "precompute-radiance-transport/prt.vert",
                 EXAMPLES + "precompute-radiance-transport/prt.frag")
    {
        uniform_attrs_location_init();
    }
};