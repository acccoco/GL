#pragma once

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


/// 创建一个 VAO 对象
static GLuint vao(const std::vector<float> &data, const std::vector<unsigned int> &indices = {});


struct Mesh {
private:
    GLuint vao_id = 0;
    GLsizei vertex_cnt = 0;
    bool has_ebo = true;

public:
    /// 创建没有 EBO 的模型，顶点属性包括 pos normal texcoord
    explicit Mesh(const std::vector<float> &vertices)
        : vao_id(vao(vertices)),
          vertex_cnt(static_cast<GLsizei>(vertices.size() / 8)),
          has_ebo(false)
    {}

    /// 创建有 EBO 的模型，顶点属性包括 pos normal texcoord
    Mesh(const std::vector<float> &vertices, const std::vector<unsigned int> &faces)
        : vao_id(vao(vertices, faces)),
          vertex_cnt(static_cast<GLsizei>(vertices.size()) / 8),
          has_ebo(true)
    {}

    /// 绑定 VAO，绘制模型
    void draw() const;

};


/// =================================================================

void Mesh::draw() const {
    glBindVertexArray(vao_id);
    if (has_ebo)
        glDrawElements(GL_TRIANGLES, vertex_cnt, GL_UNSIGNED_INT, nullptr);
    else
        glDrawArrays(GL_TRIANGLES, 0, vertex_cnt);
}


static GLuint vao(const std::vector<float> &data, const std::vector<unsigned int> &indices)
{
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // array buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size() * sizeof(float)), &data[0], GL_STATIC_DRAW);

    // vertex attribute
    // attribute: 0-positon, 1-normal, 2-uv
    // params: index, size, type, normalized, stride, pointer
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLsizei(8 * sizeof(float)), (void *) nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, GLsizei(8 * sizeof(float)), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, GLsizei(8 * sizeof(float)), (void *) (6 * sizeof(float)));

    // ebo
    if (!indices.empty()) {
        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                     &indices[0], GL_STATIC_DRAW);
    }

    // unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return VAO;
}
