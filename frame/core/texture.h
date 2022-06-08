#pragma once

#include <map>
#include <string>

#include <stb_image.h>

#include "misc.h"


GLuint load_texture(const std::string &file_path);

/**
 * @brief 纹理资源管理
 *
 * 如果多个模型引用同一份纹理，这个类可以避免重复读取文件。
 * 通过静态 map 实现资源管理
 */
class TextureManager
{
    TextureManager() = default;
    inline static std::map<std::string, GLuint> m;

public:
    static GLuint load_texture_(const std::string &file_path)
    {
        if (m.find(file_path) == m.end())
            m[file_path] = load_texture(file_path);
        return m[file_path];
    }
};


/// opengl 的 external format
struct GLExternalFormat {
    GLenum format;
    GLenum type;
};

/// opengl internal format 和 external format 的对应表
inline static std::unordered_map<GLint, GLExternalFormat> gl_format_lut = {
        {GL_RGB32F, {.format = GL_RGB, .type = GL_FLOAT}},
        {GL_RGB16F, {.format = GL_RGB, .type = GL_FLOAT}},
        {GL_RGBA32F, {.format = GL_RGBA, .type = GL_FLOAT}},
        {GL_RGBA16F, {.format = GL_RGBA, .type = GL_FLOAT}},
        {GL_R32F, {.format = GL_RED, .type = GL_FLOAT}},
        {GL_RG32F, {.format = GL_RG, .type = GL_FLOAT}},
};


/// 指定 texture 2D 的信息，用于创建 texture 2D
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
/// 新建一个 2d 纹理
GLuint new_tex2d(const Tex2DInfo &info);


/// cubemap 各个面的路径
struct CubeMapPath {
    std::string pos_x;
    std::string neg_x;
    std::string pos_y;
    std::string neg_y;
    std::string pos_z;
    std::string neg_z;
};
/// 从文件中读取 cubemap
GLuint load_cube_map(const CubeMapPath &tex_path);

/// 创建 depth render buffer
GLuint create_depth_buffer(GLsizei width = 1024, GLsizei height = 1024);


/**
 * 指定 cube map 的信息，用于创建
 * @field internal_format GLRGB16F, ...
 * @field external_format GL_RGB, ...
 * @field external_type GL_UNSIGNED_TYTE, ...
 * @field wrap GL_REPEAT, ...
 * @field filter GL_LINEAR
 * @field mipmap 是否生成 mipmap（预留空间）
 */
struct TexCubeInfo {
    GLsizei size{};
    GLint   internal_format{};
    GLenum  external_format{};
    GLenum  external_type{};
    GLint   wrap       = GL_CLAMP_TO_EDGE;
    GLint   min_filter = GL_LINEAR;
    GLint   mag_filter = GL_LINEAR;
    bool    mip_map    = false;
};

/// 创建一个空的 cube map
GLuint new_cubemap(const TexCubeInfo &info);

