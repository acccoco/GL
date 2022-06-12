#include "../shader.h"
#include <spdlog/spdlog.h>
#include "../misc.h"
#include "../opengl-misc.h"


void UniformAttribute2::init_uni_attr_func_list()
{
    uni_attr_func_list[UniAttrType::INT] = [](GLint l, UniAttrValue value_ptr) {
        glUniform1i(l, value_ptr._int);
    };
    uni_attr_func_list[UniAttrType::FLOAT] = [](GLint l, UniAttrValue value_ptr) {
        glUniform1f(l, value_ptr._float);
    };
    uni_attr_func_list[UniAttrType::MAT3] = [](GLint l, UniAttrValue v) {
        glUniformMatrix3fv(l, 1, GL_FALSE, glm::value_ptr(v._mat3));
    };
    uni_attr_func_list[UniAttrType::MAT4] = [](GLint l, UniAttrValue v) {
        glUniformMatrix4fv(l, 1, GL_FALSE, glm::value_ptr(v._mat4));
    };
    uni_attr_func_list[UniAttrType::VEC3] = [](GLint l, UniAttrValue v) {
        glUniform3fv(l, 1, glm::value_ptr(v._vec3));
    };
    uni_attr_func_list[UniAttrType::VEC4] = [](GLint l, UniAttrValue v) {
        glUniform4fv(l, 1, glm::value_ptr(v._vec4));
    };
}


void Shader2::set_uniform(const std::vector<UniformAttribute2> &attrs)
{
    glUseProgram(program_id);

    for (auto &attr: attrs)
    {
        /// 根据 uniform attribute 的 name，得到在当前 shader program 中的 location
        GLint location;
        {
            auto location_ptr = uniform_location_lut.find(attr.name);
            if (location_ptr == uniform_location_lut.end())
            {
                location = glGetUniformLocation(program_id, attr.name.c_str());
                if (location == -1)
                    LOG_AND_THROW("no uniform attribute named: {} was found.", attr.name);
                uniform_location_lut.emplace(attr.name, location);
            } else
                location = location_ptr->second;
        }
        UniformAttribute2::set(location, attr.value, attr.type);
    }
}
