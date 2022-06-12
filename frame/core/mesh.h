#pragma once

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "./misc.h"
#include "./opengl-misc.h"
#include "./material.h"


/**
 * 顶点属性对应的 slot
 */
const struct {
    GLuint pos     = 0;
    GLuint normal  = 1;
    GLuint tex_0   = 2;
    GLuint tangent = 3;
} VERTEX_ATTRBUTE_SLOT;


struct Mesh2 {
    GLuint      vao{};
    std::string name;
    GLint       primitive_mode{};          // TRIANGLE, LINE, POINT
    size_t      index_cnt{};               // 索引元素的数量
    int         index_component_type{};    // index 数据的类型，UNSIGNED_INT ...
    size_t index_offset{};    // 相对于 EBO 起始位置的偏移，可能多个 mesh 共享 ebo

    Material mat;    // mesh 的 material 信息

    /**
     * 绘制 VAO
     */
    void draw() const;
};
