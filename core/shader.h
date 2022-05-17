/**
 * 声明 uniform attribute 时，会将其注册到对应的 Shader 中\n
 * Shader 在构造函数中会读取 uniform attribute 的列表，获取他们的 location\n\n
 *
 * 建立一个 <uniform attribute type, func> 的函数查找表\n
 * 只需要知道 uniform attribute 的类型，就能得到对应的设置函数
 */

#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <functional>

#include <glad/glad.h>


/// uniform attribute 的类型
enum UniAttrType {
    INT = 0,
    FLOAT,
    MAT4,
    MAT3,
    VEC3,
    /// 确保这个变量在最后一个位置，表示类型的总数
    _total_
};

/// uniform attribute 的值，使用联合体，用来包装各种类型
union UniAttrValue {
    int       _int;
    float     _float;
    glm::mat4 _mat4;
    glm::mat3 _mat3;
    glm::vec3 _vec3;
};


/// 为 uniform attribute 设置值的函数列表
class UniAttrFuncList
{
public:
    /// 为 uniform 变量设置值的函数类型
    typedef void (*func_type)(GLint, UniAttrValue);

    /// 获得 uniform attribute 设置函数的查找表
    static func_type *get_list()
    {
        if (!has_init)
            init_uni_attr_func_list();
        return uni_attr_func_list;
    }

private:
    /// 为 uniform attribute 设置值的函数列表
    inline static func_type uni_attr_func_list[UniAttrType::_total_] = {};

    /// 函数查找表 uni_attr_func_list 是否初始化
    inline static bool has_init = false;

    /// 填充为 uniform attribute 设置值的函数的查找表
    static void init_uni_attr_func_list();
};


class Shader;
struct UniformAttribute {
public:
    /// 当前 uniform attribute 在 shader 中的变量名称
    std::string name;
    /// 当前 uniform attribute 在 shader 中的 location
    GLint location{-1};
    /// 当前 uniform attribute 的类型
    UniAttrType uni_type = UniAttrType::INT;

    /// 创建一个 uniform attribute 对象，将其注册到指定的 shader 中
    UniformAttribute(std::string name_, Shader *shader, UniAttrType type);
};


/// 从文件读取着色器文本，编译成着色器对象
GLuint shader_compile(const std::string &file_path, GLenum shader_type);

/// 链接多个着色器对象，变成 program
GLuint shader_link(GLuint vertex, GLuint fragment, GLuint geometry = 0);


class Shader
{
public:
    Shader(const std::string &vertex_shader, const std::string &fragment_shader)
    {
        program_id = shader_link(shader_compile(vertex_shader, GL_VERTEX_SHADER),
                                 shader_compile(fragment_shader, GL_FRAGMENT_SHADER));

        uni_attr_func_list = UniAttrFuncList::get_list();
    }

    void use() const { glUseProgram(program_id); }

    /// 供 uniform attribute 调用，将 uniform attribute 注册到当前 shader 中
    void register_uniform_attribute(UniformAttribute *attr) { attrs.push_back(attr); }

    struct UniAttrAssign {
        UniformAttribute attr;
        UniAttrValue     value_ptr{};
    };
    /// 设置指定 uniform attribute 的值
    void set_uniform(const std::vector<UniAttrAssign> &uni_attr_assign_list) const;

protected:
    GLuint program_id;

    /// 着色器的 uniform attribute 列表
    std::vector<UniformAttribute *> attrs;

    /// uniform attribute 设置函数的查找表
    UniAttrFuncList::func_type *uni_attr_func_list;

    /// 为所有的 uniform attribute 找到在 shader program 中的 location
    void uniform_attrs_location_init();
};


///===================================================================

inline UniformAttribute::UniformAttribute(std::string name_, Shader *shader, UniAttrType type)
    : name(std::move(name_)),
      uni_type(type)
{
    shader->register_uniform_attribute(this);
}

inline GLuint shader_compile(const std::string &file_path, GLenum shader_type)
{
    // read file
    std::fstream      fs;
    std::stringstream ss;
    fs.open(file_path, std::ios::in);
    if (!fs.is_open())
        std::cout << "fail to open file: " << file_path << std::endl;
    for (std::string str; std::getline(fs, str); ss << str << '\n')
        ;
    fs.close();
    std::string shader_str   = ss.str();
    auto        shader_c_str = shader_str.c_str();

    // compile shader
    GLuint shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &shader_c_str, nullptr);
    glCompileShader(shader_id);

    // check
    int  success;
    char info[512];
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader_id, 512, nullptr, info);
        std::cout << "shader compile error: " << file_path << std::endl;
        std::cout << info << std::endl;
    }

    return shader_id;
}

inline void Shader::uniform_attrs_location_init()
{
    for (auto attr: attrs)
    {
        attr->location = glGetUniformLocation(program_id, attr->name.c_str());
        if (attr->location == -1)
            std::cout << "fail to get uniform location, name: " << attr->name.c_str() << std::endl;
    }
}

void Shader::set_uniform(const std::vector<UniAttrAssign> &uni_attr_assign_list) const
{
    this->use();
    for (auto &assign: uni_attr_assign_list)
    {
        uni_attr_func_list[static_cast<int>(assign.attr.uni_type)](assign.attr.location, assign.value_ptr);
    }
}

inline GLuint shader_link(GLuint vertex, GLuint fragment, GLuint geometry)
{
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex);
    glAttachShader(program_id, fragment);
    if (geometry)
        glAttachShader(program_id, geometry);
    glLinkProgram(program_id);

    // check
    int  success;
    char info[512];
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program_id, 512, nullptr, info);
        std::cout << "shader link error." << std::endl;
        std::cout << info << std::endl;
    }
    return program_id;
}

inline void UniAttrFuncList::init_uni_attr_func_list()
{
    uni_attr_func_list[UniAttrType::INT] = [](GLint location, UniAttrValue value_ptr) {
        glUniform1i(location, value_ptr._int);
    };
    uni_attr_func_list[UniAttrType::FLOAT] = [](GLint location, UniAttrValue value_ptr) {
        glUniform1f(location, value_ptr._float);
    };
    uni_attr_func_list[UniAttrType::MAT3] = [](GLint location, UniAttrValue value) {
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value._mat3));
    };
    uni_attr_func_list[UniAttrType::MAT4] = [](GLint location, UniAttrValue value) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value._mat4));
    };
    uni_attr_func_list[UniAttrType::VEC3] = [](GLint location, UniAttrValue value) {
        glUniform3fv(location, 1, glm::value_ptr(value._vec3));
    };
}
