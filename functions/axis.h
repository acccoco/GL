#pragma once

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fmt/format.h>

#include "config.hpp"
#include "core/shader2.h"


class Axis
{
public:
    Axis() { this->vao = get_axis_vao(); }

    void draw(const glm::mat4 &camera_mvp);

private:
    static GLuint get_axis_vao();

private:
    Shader2 shader_line = Shader2(SHADER + "line.vert", SHADER + "line.frag");

    GLuint vao;

    /// axis 模型顶点的数量
    static constexpr GLint vertex_cnt = 6;

    /// 绘制 axis 的顶点数据 (x, y, z, R, G, B)
    static constexpr float axis[] = {
            // x axis
            0.f, 0.f, 0.f, 1.f, 0.f, 0.f,    // origin
            3.f, 0.f, 0.f, 1.f, 0.f, 0.f,    // +x
            // y axis
            0.f, 0.f, 0.f, 0.f, 1.f, 0.f,    // origin
            0.f, 3.f, 0.f, 0.f, 1.f, 0.f,    // +y
            // z axis
            0.f, 0.f, 0.f, 0.f, 0.f, 1.f,    // origin
            0.f, 0.f, 3.f, 0.f, 0.f, 1.f,    // +z
    };
};


/// ==================================================================


inline GLuint Axis::get_axis_vao()
{
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (nullptr));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));

    /// unbind
    glBindVertexArray(0);

    return VAO;
}

inline void Axis ::draw(const glm::mat4 &camera_mvp)
{
    shader_line.set_uniform({
            {"u_camera_mvp", MAT4, {._mat4 = camera_mvp}},
    });
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, vertex_cnt);
}