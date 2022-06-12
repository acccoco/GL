#pragma once

#include <array>
#include <vector>

#include <glad/glad.h>

#include "./misc.h"


/**
 * 使用 OpenGL 的接口检查错误信息
 */
void check_gl_error(const char *file, int line);


#define CHECK_GL_ERROR() check_gl_error(__FILE_NAME__, __LINE__)


/**
 * 为 framebuffer 绑定 depth 和 color 附件
 */
void framebuffer_bind(GLuint framebuffer, GLuint depth_buffer,
                      const std::vector<GLuint> &color_attachment_list);


/**
 * 纹理绑定的简化写法
 * @param target GL_TEXTURE_2D, ...
 * @param texture_loc 0, 1, 2, ...
 * @param texture texture id
 */
void glBindTexture_(GLenum target, int texture_loc, GLuint texture);


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
void glViewport_(GLsizei framebuffer_width, GLsizei framebuffer_height, GLint xcnt, GLint ycnt,
                 GLint xidx, GLint yidx, GLint xlen = 1, GLint ylen = 1);


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
void glViewport_(const ViewPortInfo &info);


/**
 * 创建一个 VAO 对象，
 * @param data 顶点数据，需要包括 position, normal, texcoord，格式如下：\n
 *             | position0 | normal0 | texcoord0 | position1 | normal2 | ...
 * @param indices EBO 数据，这是可选的
 * @return
 */
GLuint vao(const std::vector<float> &data, const std::vector<unsigned int> &indices = {});


/**
 * 将场景绘制到 cubemap 时摄像机的朝向
 */
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


/**
 * opengl 的 external format
 */
struct GLExternalFormat {
    GLenum format;
    GLenum type;
};


/**
 * opengl internal format 和 external format 的对应表
 */
inline static std::unordered_map<GLint, GLExternalFormat> gl_format_lut = {
        {GL_RGB32F, {.format = GL_RGB, .type = GL_FLOAT}},
        {GL_RGB16F, {.format = GL_RGB, .type = GL_FLOAT}},
        {GL_RGBA32F, {.format = GL_RGBA, .type = GL_FLOAT}},
        {GL_RGBA16F, {.format = GL_RGBA, .type = GL_FLOAT}},
        {GL_R32F, {.format = GL_RED, .type = GL_FLOAT}},
        {GL_RG32F, {.format = GL_RG, .type = GL_FLOAT}},
};


/**
 * 指定 texture 2D 的信息，用于创建 texture 2D
 */
struct Tex2DInfo {
    GLsizei width{}, height{};

    GLint  internal_format{};    // eg. GL_RGB16F
    GLenum external_format{};    // eg. GL_RED, GL_RG, GL_RGB, GL_RGBA
    GLenum external_type{};      // eg. GL_UNSIGNED_TYTE, GL_UNSIGNED_INT

    GLint wrap_s     = GL_CLAMP_TO_EDGE;    // default: GL_CLAMP_TO_EDGE
    GLint wrap_t     = GL_CLAMP_TO_EDGE;    // default: GL_CLAMP_TO_EDGE
    GLint filter_min = GL_LINEAR;           // default: GL_LINEAR
    GLint filter_mag = GL_LINEAR;           // default: GL_LINEAR

    bool mipmap{false};    // 是否生成 mipmap，default：false

    const GLvoid *data = nullptr;
};


/**
 * 创建一个 GL_TEXTURE_2D 的纹理
 */
GLuint new_tex2d(const Tex2DInfo &info);


/**
 * 创建 depth render buffer
 */
GLuint create_depth_buffer(GLsizei width = 1024, GLsizei height = 1024);


/**
 * 指定 cube map 的信息，用于创建 cubemap
 */
struct TexCubeInfo {
    GLsizei size{};
    GLint   internal_format{};                // eg. GLRGB16F, ...
    GLenum  external_format{};                // eg. GL_RGB, ...
    GLenum  external_type{};                  // eg. GL_UNSIGNED_TYTE, ...
    GLint   wrap       = GL_CLAMP_TO_EDGE;    // default: GL_CLAMP_TO_EDGE
    GLint   min_filter = GL_LINEAR;           // default: GL_LINEAR
    GLint   mag_filter = GL_LINEAR;           // default: GL_LINEAR
    bool    mip_map    = false;               // default: false

    std::array<GLvoid *, 6> data{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
};


/**
 * 创建一个 cube map，用指定的数据填入
 */
GLuint new_cubemap(const TexCubeInfo &info);


/**
 * 从文件读取着色器文本，编译成着色器对象
 * @param shader_type 着色器类型，可以是 GL_FRAGMENT_SHADER 或 GL_VERTEX_SHADER
 */
GLuint shader_compile(const std::string &file_path, GLenum shader_type);


/**
 * 链接多个着色器对象，变成 program
 */
GLuint shader_link(GLuint vertex, GLuint fragment, GLuint geometry = 0);