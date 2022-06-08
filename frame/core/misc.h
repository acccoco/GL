#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>


/// 使用 spdlog 打印错误日志，并且抛出异常
#define LOG_AND_THROW(str, args...)                                                                \
    do                                                                                             \
    {                                                                                              \
        SPDLOG_ERROR(str, ##args);                                                                 \
        throw std::exception();                                                                    \
    } while (false)


/// OpenGL 错误检查
inline void check_gl_error(const char *file, int line)
{
    GLenum error_code;
    /// 可能会设置多个 error flags，因此需要循环调用，直到返回 GL_NO_ERROR。每调用一次就清除一个 error flag
    while ((error_code = glGetError()) != GL_NO_ERROR)
    {
        std::string error_str;

#define LOCAL_ERROR_BRANCH(err)                                                                    \
    case err: error_str = #err; break;

        switch (error_code)
        {
            LOCAL_ERROR_BRANCH(GL_INVALID_ENUM)
            LOCAL_ERROR_BRANCH(GL_INVALID_VALUE)
            LOCAL_ERROR_BRANCH(GL_INVALID_OPERATION)
            LOCAL_ERROR_BRANCH(GL_INVALID_FRAMEBUFFER_OPERATION)
            LOCAL_ERROR_BRANCH(GL_OUT_OF_MEMORY)
            default: error_str = "unknow error code: " + std::to_string(error_code);
        }

#undef GL_ERROR_BRANCH

        spdlog::error("[{}:{}]OpenGL error: {}", file, line, error_str);
    }
}
#define CHECK_GL_ERROR() check_gl_error(__FILE_NAME__, __LINE__)


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


inline void glViewport_(GLint x, GLint y, GLsizei width, GLsizei height)
{
    glViewport(x, y, width, height);
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
inline void glViewport_(GLsizei width, GLsizei height, GLint xcnt, GLint ycnt, GLint xidx,
                        GLint yidx, GLint xlen = 1, GLint ylen = 1)
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


/// 为 framebuffer 绑定 depth 和 color 附件
inline void framebuffer_bind(GLuint framebuffer, GLuint depth_buffer,
                             const std::vector<GLuint> &color_attachment_list)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    /// depth attachment
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

    /// color attachments
    size_t              color_attachment_cnt = color_attachment_list.size();
    std::vector<GLenum> frag_out_layout;
    for (size_t i = 0; i < color_attachment_cnt; ++i)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color_attachment_list[i], 0);
        frag_out_layout.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    /// 是否使用多目标渲染
    if (color_attachment_cnt > 1)
    {
        glDrawBuffers(static_cast<int>(color_attachment_cnt), frag_out_layout.data());
    }

    /// check
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        SPDLOG_ERROR("geometry pass framebuffer incomplete.");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


template<typename T>
inline bool is_one_of(T a, const std::vector<T> &list)
{
    for (const T &b: list)
        if (a == b)
            return true;
    return false;
}
