#include "texture.h"


GLuint new_tex2d(const Tex2DInfo &info)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    GLenum external_format = info.external_format;
    GLenum external_type   = info.external_type;
    if (external_format == 0 || external_type == 0)
    {
        /// 根据 internal format 查询 external format
        auto external_format_iter = gl_format_lut.find(info.internal_format);
        if (external_format_iter == gl_format_lut.end())
            LOG_AND_THROW("unknow internal format: {}", info.internal_format);
        external_format = external_format_iter->second.format;
        external_type   = external_format_iter->second.type;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, info.internal_format, info.width, info.height, 0,
                 external_format, external_type, info.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, info.wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, info.wrap_t);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, info.filter_min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, info.filter_mag);

    if (info.mipmap)
        glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERROR();
    return texture_id;
}

GLuint load_texture(const std::string &file_path)
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
        case 1:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
            break;
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                         data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         data);
            break;
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

    CHECK_GL_ERROR();
    return texture_id;
}


GLuint load_cube_map(const CubeMapPath &tex_path)
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
        glTexImage2D(tex_target, 0, internal_format, width, height, 0, internal_format,
                     GL_UNSIGNED_BYTE, data);
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

    CHECK_GL_ERROR();
    return cube_map;
}

GLuint create_depth_buffer(GLsizei width, GLsizei height)
{
    GLuint render_buffer;
    glGenRenderbuffers(1, &render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CHECK_GL_ERROR();
    return render_buffer;
}


GLuint new_cubemap(const TexCubeInfo &info)
{
    GLuint cube_map;
    glGenTextures(1, &cube_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);

    // order: +x, -x, +y, -y, +z, -z
    for (int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, info.internal_format, info.size,
                     info.size, 0, info.external_format, info.external_type, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, info.wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, info.wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, info.wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, info.min_filter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, info.mag_filter);

    if (info.mip_map)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    CHECK_GL_ERROR();
    return cube_map;
}
