/**
 * 声明 uniform attribute 时，会将其注册到对应的 Shader 中\n
 * Shader 在构造函数中会读取 uniform attribute 的列表，获取他们的 location\n\n
 *
 * 建立一个 <uniform attribute type, func> 的函数查找表\n
 * 只需要知道 uniform attribute 的类型，就能得到对应的设置函数
 */

#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <utility>
#include <functional>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


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
    [[deprecated]] UniformAttribute(std::string name_, Shader *shader, UniAttrType type);
};


/// 从文件读取着色器文本，编译成着色器对象
GLuint shader_compile(const std::string &file_path, GLenum shader_type);

/// 链接多个着色器对象，变成 program
GLuint shader_link(GLuint vertex, GLuint fragment, GLuint geometry = 0);


class Shader
{
public:
    [[deprecated("use Shader2 class")]] Shader(const std::string &vertex_shader, const std::string &fragment_shader)
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
