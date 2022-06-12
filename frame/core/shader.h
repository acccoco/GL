/**
 * 声明 uniform attribute 时，会将其注册到对应的 Shader 中\n
 * Shader 在构造函数中会读取 uniform attribute 的列表，获取他们的 location\n\n
 *
 * 建立一个 <uniform attribute type, func> 的函数查找表\n
 * 只需要知道 uniform attribute 的类型，就能得到对应的设置函数
 */

#pragma once

#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>
#include <exception>
#include <functional>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include "./opengl-misc.h"


/**
 * uniform attribute 的类型
 */
enum UniAttrType {
    INT = 0,
    FLOAT,
    MAT4,
    MAT3,
    VEC3,
    VEC4,
    /// 确保这个变量在最后一个位置，表示类型的总数
    _total_
};


/**
 * 包装所有 Uniform Attribute，设置各种 Uniform Attribute 时可以使用同一个函数接口
 */
union UniAttrValue {
    int       _int;
    float     _float;
    glm::mat4 _mat4;
    glm::mat3 _mat3;
    glm::vec3 _vec3;
    glm::vec4 _vec4;
};


/**
 * 完整的 uniform attribute，包括 type 和 value
 */
struct UniformAttribute2 {
    std::string  name;
    UniAttrType  type;
    UniAttrValue value;

    UniformAttribute2(std::string n, UniAttrType t, const UniAttrValue &v)
        : name(std::move(n)), type(t), value(v)
    {}
    UniformAttribute2(std::string n, int v) : name(std::move(n)), type(INT), value{._int = v} {}
    UniformAttribute2(std::string n, float v)
        : name(std::move(n)), type(FLOAT), value({._float = v})
    {}
    UniformAttribute2(std::string n, const glm::vec3 &v)
        : name(std::move(n)), type(VEC3), value({._vec3 = v})
    {}
    UniformAttribute2(std::string n, const glm::mat3 &v)
        : name(std::move(n)), type(MAT3), value({._mat3 = v})
    {}
    UniformAttribute2(std::string n, const glm::mat4 &v)
        : name(std::move(n)), type(MAT4), value({._mat4 = v})
    {}
    UniformAttribute2(std::string n, const glm::vec4 &v)
        : name(std::move(n)), type(VEC4), value({._vec4 = v})
    {}

    static void set(GLint location, UniAttrValue value, UniAttrType type)
    {
        static bool func_list_init = false;
        if (!func_list_init)
        {
            init_uni_attr_func_list();
            func_list_init = true;
        }
        uni_attr_func_list[type](location, value);
    }


private:
    typedef void (*func_type)(GLint, UniAttrValue);


    /**
     * 为 uniform attribute 设置值的函数列表
     */
    inline static func_type uni_attr_func_list[UniAttrType::_total_] = {};

    /**
     * 初始化为 uniform attribute 设置值的函数的查找表
     */
    static void init_uni_attr_func_list();
};


class Shader2
{
public:
    Shader2(const std::string &vert, const std::string &frag)
    {
        program_id = shader_link(shader_compile(vert, GL_VERTEX_SHADER),
                                 shader_compile(frag, GL_FRAGMENT_SHADER));
    }

    void set_uniform(const std::vector<UniformAttribute2> &attrs);

    void use() const { glUseProgram(program_id); }

    [[nodiscard]] GLuint get_program_id() const { return program_id; }


private:
    GLuint program_id;

    /**
     * 缓存当前 shader program 中的 uniform attribute location
     */
    std::unordered_map<std::string, GLint> uniform_location_lut;
};
