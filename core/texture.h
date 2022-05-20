#pragma once

#include <map>
#include <string>

#include <stb_image.h>


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

/**
 * 指定 texture 2D 的信息，用于创建 texture 2D
 * @field internal_format GLRGB16F, ...
 * @field external_format GL_RGB, ...
 * @field external_type GL_UNSIGNED_TYTE, ...
 * @field wrap GL_REPEAT, ...
 * @field filter GL_LINEAR
 */
struct Tex2DInfo {
    GLsizei width{}, height{};
    GLint   internal_format{};
    GLenum  external_format{};
    GLenum  external_type{};
    GLint   wrap   = GL_CLAMP_TO_EDGE;
    GLint   filter = GL_LINEAR;
};
/// 新建一个空的 2d 纹理
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


/// =================================================================


inline GLuint new_tex2d(const Tex2DInfo &info)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, info.internal_format, info.width, info.height, 0, info.external_format,
                 info.external_type, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, info.wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, info.wrap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, info.filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, info.filter);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture_id;
}

inline GLuint load_texture(const std::string &file_path)
{
    /// read file
    int width, height, channels;

    SPDLOG_INFO("load texture: {}...", file_path);

    /// flip vertical, make sure bottom left is uv(0, 0)
    stbi_set_flip_vertically_on_load(true);
    auto data = stbi_load(file_path.c_str(), &width, &height, &channels, 0);
    if (!data)
        SPDLOG_WARN("error on load texture.");


    /// create texture and bind
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    switch (channels)
    {
        case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data); break;
        case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); break;
        case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); break;
        default: SPDLOG_ERROR("bad texture channes: {}", channels);
    }

    /// repeat sample
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /// scale sample
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /// mipmap
    glGenerateMipmap(GL_TEXTURE_2D);

    /// unbind and close file
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    return texture_id;
}


inline GLuint load_cube_map(const CubeMapPath &tex_path)
{
    GLuint cube_map;
    glGenTextures(1, &cube_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);

    auto load_set = [&](const std::string &path, GLenum tex_target) {
        int width, height, channels;
        // DO NOT flip vertical for historical reason.
        stbi_set_flip_vertically_on_load(false);
        auto data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data)
            SPDLOG_ERROR("error on load texture, path: {}", path);
        if (channels != 3 && channels != 4)
            SPDLOG_ERROR("tex channels error, path: {}", path);
        GLint internal_format = channels == 3 ? GL_RGB : GL_RGBA;
        glTexImage2D(tex_target, 0, internal_format, width, height, 0, internal_format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    };

    SPDLOG_INFO("load cube map texture: {}", tex_path.pos_x);
    load_set(tex_path.pos_x, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    load_set(tex_path.neg_x, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
    load_set(tex_path.pos_y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    load_set(tex_path.neg_y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
    load_set(tex_path.pos_z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
    load_set(tex_path.neg_z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

    /* 多级纹理 */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    /* uv 超过后如何采样 */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cube_map;
}

inline GLuint create_depth_buffer(GLsizei width, GLsizei height)
{
    GLuint render_buffer;
    glGenRenderbuffers(1, &render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return render_buffer;
}


inline GLuint new_cubemap(const TexCubeInfo &info)
{
    GLuint cube_map;
    glGenTextures(1, &cube_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);

    // order: +x, -x, +y, -y, +z, -z
    for (int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, info.internal_format, info.size, info.size, 0,
                     info.external_format, info.external_type, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, info.wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, info.wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, info.wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, info.min_filter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, info.mag_filter);

    if (info.mip_map)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return cube_map;
}
