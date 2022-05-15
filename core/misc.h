#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


const glm::vec3 POSITIVE_X = {1, 0, 0};
const glm::vec3 NEGATIVE_X = {-1, 0, 0};
const glm::vec3 POSITIVE_Y = {0, 1, 0};
const glm::vec3 NEGATIVE_Y = {0, -1, 0};
const glm::vec3 POSITIVE_Z = {0, 0, 1};
const glm::vec3 NEGATIVE_Z = {0, 0, -1};


inline void glViewport_(GLint x, GLint y, GLsizei width, GLsizei height)
{
#ifdef __APPLE__
    glViewport(x * 2, y * 2, width * 2, height * 2);
#else
    glViewport(x, y, width, height);
#endif
}

/**
 * 设置屏幕上的显示区域
 * @param width 窗口的宽度
 * @param height 窗口的长度
 * @param xcnt 宽分为几份
 * @param ycnt 长分为几份
 * @param xidx x 从第几个开始
 * @param yidx y 从第几个开始
 * @param xlen 占据几个格子宽
 * @param ylen 占据几个格子高
 */
inline void glViewport_(GLsizei width, GLsizei height, GLint xcnt, GLint ycnt, GLint xidx, GLint yidx, GLint xlen = 1,
                        GLint ylen = 1)
{
    auto x_delta  = width / xcnt;
    auto y_delta  = height / ycnt;
    auto x_width  = x_delta * xlen;
    auto y_height = y_delta * ylen;
#ifdef __APPLE__
    glViewport(xidx * x_delta * 2, yidx * y_delta * 2, x_width * 2, y_height * 2);
#else
    glViewport(xidx * x_delta, yidx * y_delta, x_width, y_height);
#endif
}


/**
 * 纹理绑定的简化写法
 * @param target GL_TEXTURE_2D, ...
 * @param texture_loc 0, 1, 2, ...
 * @param texture texture id
 */
inline void glBindTexture_(GLenum target, int texture_loc, GLuint texture)
{
    glActiveTexture(GL_TEXTURE0 + texture_loc);
    glBindTexture(target, texture);
}