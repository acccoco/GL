#pragma once

#include <fstream>
#include <iostream>
#include <sstream>

#include <glad/glad.h>


class UniformAttribute
{
public:
    std::string name;
    GLuint program_id{0};
    GLint location{-1};
    explicit UniformAttribute(std::string name_)
        : name(std::move(name_))
    {}
    void init_location(GLuint program_id_)
    {
        program_id = program_id_;
        location = glGetUniformLocation(program_id, name.c_str());
        if (location == -1)
            std::cout << "fail to get uniform location, name: " << name << std::endl;
    }
};

class UniformAttribute1i : public UniformAttribute
{
public:
    explicit UniformAttribute1i(const std::string &name)
        : UniformAttribute(name)
    {}

    void set(int value)
    {
        glUseProgram(program_id);
        if (location == -1)
            std::cout << "uniform location error: " << name << std::endl;
        glUniform1i(location, value);
    }
};

class UniformAttributeM4fv : public UniformAttribute
{
public:
    explicit UniformAttributeM4fv(const std::string &name)
        : UniformAttribute(name)
    {}
    void set(const glm::mat4 &value)
    {
        glUseProgram(program_id);
        if (location == -1)
            std::cout << "uniform location error: " << name << std::endl;
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }
};

class UniformAttribute3fv : public UniformAttribute
{
public:
    explicit UniformAttribute3fv(const std::string &name)
        : UniformAttribute(name)
    {}
    void set(const glm::vec3 &value)
    {
        glUseProgram(program_id);
        if (location == -1)
            std::cout << "uniform location error: " << name << std::endl;
        glUniform3fv(location, 1, glm::value_ptr(value));
    }
};

class UniformAttribute1f : public UniformAttribute
{
public:
    explicit UniformAttribute1f(const std::string &name)
        : UniformAttribute(name)
    {}
    void set(float value)
    {
        glUseProgram(program_id);
        if (location == -1)
            std::cout << "uniform location error: " << name << std::endl;
        glUniform1f(location, value);
    }
};

static GLuint shader_compile(const std::string &file_path, GLenum shader_type);
static GLuint shader_link(GLuint vertex, GLuint fragment, GLuint geometry = 0);

class Shader
{
protected:
    GLuint program_id;

public:
    Shader(const std::string &vertex_shader, const std::string &fragment_shader)
    {
        program_id = shader_link(shader_compile(vertex_shader, GL_VERTEX_SHADER),
                                 shader_compile(fragment_shader, GL_FRAGMENT_SHADER));
    }
};

static GLuint shader_compile(const std::string &file_path, GLenum shader_type)
{
    // read file
    std::fstream fs;
    std::stringstream ss;
    fs.open(file_path, std::ios::in);
    if (!fs.is_open())
        std::cout << "fail to open file: " << file_path << std::endl;
    for (std::string str; std::getline(fs, str); ss << str << '\n')
        ;
    fs.close();
    std::string shader_str = ss.str();
    auto shader_c_str = shader_str.c_str();

    // compile shader
    GLuint shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &shader_c_str, nullptr);
    glCompileShader(shader_id);

    // check
    int success;
    char info[512];
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader_id, 512, nullptr, info);
        std::cout << "shader compile error: " << file_path << std::endl;
        std::cout << info << std::endl;
    }

    return shader_id;
}

static GLuint shader_link(GLuint vertex, GLuint fragment, GLuint geometry)
{
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex);
    glAttachShader(program_id, fragment);
    if (geometry)
        glAttachShader(program_id, geometry);
    glLinkProgram(program_id);

    // check
    int success;
    char info[512];
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program_id, 512, nullptr, info);
        std::cout << "shader link error." << std::endl;
        std::cout << info << std::endl;
    }
    return program_id;
}
