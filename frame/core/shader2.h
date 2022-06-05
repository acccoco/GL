#pragma once

#include <utility>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>
#include <exception>
#include <functional>
#include <unordered_map>

#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include "core/shader.h"


struct UniformAttribute2 {
    std::string  name;
    UniAttrType  type;
    UniAttrValue value;

    UniformAttribute2(std::string n, UniAttrType t, const UniAttrValue &v) : name(std::move(n)), type(t), value(v) {}
    UniformAttribute2(std::string n, int v) : name(std::move(n)), type(INT), value{._int = v} {}
    UniformAttribute2(std::string n, float v) : name(std::move(n)), type(FLOAT), value({._float = v}) {}
    UniformAttribute2(std::string n, const glm::vec3 &v) : name(std::move(n)), type(VEC3), value({._vec3 = v}) {}
    UniformAttribute2(std::string n, const glm::mat3 &v) : name(std::move(n)), type(MAT3), value({._mat3 = v}) {}
    UniformAttribute2(std::string n, const glm::mat4 &v) : name(std::move(n)), type(MAT4), value({._mat4 = v}) {}
};


class Shader2
{
public:
    Shader2(const std::string &vert, const std::string &frag)
    {
        program_id = shader_link(shader_compile(vert, GL_VERTEX_SHADER), shader_compile(frag, GL_FRAGMENT_SHADER));

        uni_attr_func_list = UniAttrFuncList::get_list();
    }


    void set_uniform(const std::vector<UniformAttribute2> &attrs)
    {
        glUseProgram(program_id);

        for (auto &attr: attrs)
        {
            auto  location_ptr = uniform_location_lut.find(attr.name);
            GLint location;

            /// lut 中没有这个 uniform attribute
            if (location_ptr == uniform_location_lut.end())
            {
                location = glGetUniformLocation(program_id, attr.name.c_str());
                if (location == -1)
                {
                    SPDLOG_ERROR("no uniform attribute named: {} was found.", attr.name);
                    throw std::exception();
                }
                uniform_location_lut.emplace(attr.name, location);
            } else
            {
                location = location_ptr->second;
            }

            uni_attr_func_list[attr.type](location, attr.value);
        }
    }

private:
    GLuint program_id;

    /// uniform name - location 的 look-up table
    std::unordered_map<std::string, GLint> uniform_location_lut;

    UniAttrFuncList::func_type *uni_attr_func_list;
};