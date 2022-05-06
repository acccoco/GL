#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.hpp"
#include "core/shader.h"
#include "core/model.h"


class ShaderPRT : public Shader
{
public:
    UniformAttributeM4fv m_model{"m_model", this};
    UniformAttributeM4fv m_view{"m_view", this};
    UniformAttributeM4fv m_proj{"m_proj", this};
    UniformAttributeM3fv SH_light_R{"SH_light_R", this};
    UniformAttributeM3fv SH_light_G{"SH_light_G", this};
    UniformAttributeM3fv SH_light_B{"SH_light_B", this};


    ShaderPRT()
        : Shader(EXAMPLES + "precompute-radiance-transport/prt.vert",
                 EXAMPLES + "precompute-radiance-transport/prt.frag")
    {
        uniform_attrs_location_init();
    }
};