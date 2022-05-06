#pragma once

#include <map>
#include <string>
#include <iostream>


#include <stb_image.h>

static GLuint load_texture(const std::string &file_path);

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
        GLuint tex_id;
        if (m.find(file_path) == m.end()) {
            tex_id = load_texture(file_path);
            m[file_path] = tex_id;
        } else
            tex_id = m[file_path];

        return tex_id;
    }
};

static GLuint load_texture(const std::string &file_path)
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

    if (channels == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    else if (channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    else
        std::cout << "channels error" << std::endl;

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

[[maybe_unused]] static GLuint load_cube_map(const std::string &positive_x, const std::string &negative_x,
                                             const std::string &positive_y, const std::string &negative_y,
                                             const std::string &positive_z, const std::string &negative_z)
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
            std::cout << "error on load texture, path: " << path << std::endl;
        if (channels != 3 && channels != 4)
            std::cout << "tex channels error, path: " << path << std::endl;
        GLint internal_format = channels == 3 ? GL_RGB : GL_RGBA;
        glTexImage2D(tex_target, 0, internal_format, width, height, 0, internal_format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    };

    SPDLOG_INFO("load cube map texture: {}", positive_x);
    load_set(positive_x, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    load_set(negative_x, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
    load_set(positive_y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    load_set(negative_y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
    load_set(positive_z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
    load_set(negative_z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

    /* 多级纹理 */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    /* uv 超过后如何采样 */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cube_map;
}

[[maybe_unused]] static GLuint create_depth_buffer(GLsizei width = 1024, GLsizei height = 1024)
{
    GLuint render_buffer;
    glGenRenderbuffers(1, &render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return render_buffer;
}

GLuint create_tex(GLsizei width = 1024, GLsizei height = 1024)
{
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return tex_id;
}


GLuint create_cube_map(GLsizei size = 1024)
{
    GLuint cube_map;

    glGenTextures(1, &cube_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);

    // order: +x, -x, +y, -y, +z, -z
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return cube_map;
}
