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


/// 绘制 cubemap 时摄像机的朝向
struct CameraDirDrawCube {
    struct CameraPosture {
        glm::vec3 up;
        glm::vec3 front;
    };

    inline const static CameraPosture pos_x = {.up = NEGATIVE_Y, .front = POSITIVE_X};
    inline const static CameraPosture neg_x = {.up = NEGATIVE_Y, .front = NEGATIVE_X};
    inline const static CameraPosture pos_z = {.up = NEGATIVE_Y, .front = POSITIVE_Z};
    inline const static CameraPosture neg_z = {.up = NEGATIVE_Y, .front = NEGATIVE_Z};
    inline const static CameraPosture pos_y = {.up = POSITIVE_Z, .front = POSITIVE_Y};
    inline const static CameraPosture neg_y = {.up = NEGATIVE_Z, .front = NEGATIVE_Y};
};


inline void glViewport_(GLint x, GLint y, GLsizei width, GLsizei height) { glViewport(x, y, width, height); }

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

    glViewport(xidx * x_delta, yidx * y_delta, x_width, y_height);
}


struct ViewPortInfo {
    GLsizei width{};
    GLsizei height{};
    GLint   x_cnt{};
    GLint   y_cnt{};
    GLint   x_idx{};
    GLint   y_idx{};
    GLint   x_len = 1;
    GLint   y_len = 1;
};
inline void glViewport_(const ViewPortInfo &info)
{
    auto x_delta  = info.width / info.x_cnt;
    auto y_delta  = info.height / info.y_cnt;
    auto x_width  = x_delta * info.x_len;
    auto y_height = y_delta * info.y_len;

    glViewport(info.x_idx * x_delta, info.y_idx * y_delta, x_width, y_height);
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


/// 合并两个 vector
template<class T>
inline void combine(std::vector<T> &a, std::vector<T> &b)
{
    a.insert(a.end(), b.begin(), b.end());
}
