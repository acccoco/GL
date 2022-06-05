#include "./shader.h"
#include <spdlog/spdlog.h>


UniformAttribute::UniformAttribute(std::string name_, Shader *shader, UniAttrType type)
    : name(std::move(name_)), uni_type(type)
{
    shader->register_uniform_attribute(this);
}

GLuint shader_compile(const std::string &file_path, GLenum shader_type)
{
    // read file
    std::fstream      fs;
    std::stringstream ss;
    fs.open(file_path, std::ios::in);
    if (!fs.is_open())
    {
        SPDLOG_ERROR("fail to open file: {}", file_path);
        throw(std::exception());
    }
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
        SPDLOG_ERROR("shader compile error: {}, info: \n{}", file_path, info);
        throw(std::exception());
    }

    return shader_id;
}

void Shader::uniform_attrs_location_init()
{
    for (auto attr: attrs)
    {
        attr->location = glGetUniformLocation(program_id, attr->name.c_str());
        if (attr->location == -1)
            SPDLOG_ERROR("fail to get uniform location, name: {}", attr->name);
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

GLuint shader_link(GLuint vertex, GLuint fragment, GLuint geometry)
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
        SPDLOG_ERROR("shader link error, info: {}", info);
        throw(std::exception());
    }
    return program_id;
}

void UniAttrFuncList::init_uni_attr_func_list()
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
