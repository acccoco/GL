#include "../opengl-misc.h"

#include <string>
#include <cassert>
#include <sstream>
#include <fstream>

#include <glad/glad.h>
#include <spdlog/spdlog.h>


void check_gl_error(const char *file, int line)
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


void framebuffer_bind(GLuint framebuffer, GLuint depth_buffer,
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

    CHECK_GL_ERROR();
}


void glBindTexture_(GLenum target, int texture_loc, GLuint texture)
{
    glActiveTexture(GL_TEXTURE0 + texture_loc);
    glBindTexture(target, texture);

    CHECK_GL_ERROR();
}


void glViewport_(GLsizei width, GLsizei height, GLint xcnt, GLint ycnt, GLint xidx, GLint yidx,
                 GLint xlen, GLint ylen)
{
    auto x_delta  = width / xcnt;
    auto y_delta  = height / ycnt;
    auto x_width  = x_delta * xlen;
    auto y_height = y_delta * ylen;

    glViewport(xidx * x_delta, yidx * y_delta, x_width, y_height);

    CHECK_GL_ERROR();
}


void glViewport_(const ViewPortInfo &info)
{
    auto x_delta  = info.width / info.x_cnt;
    auto y_delta  = info.height / info.y_cnt;
    auto x_width  = x_delta * info.x_len;
    auto y_height = y_delta * info.y_len;

    glViewport(info.x_idx * x_delta, info.y_idx * y_delta, x_width, y_height);

    CHECK_GL_ERROR();
}


GLuint vao(const std::vector<float> &data, const std::vector<unsigned int> &indices)
{
    assert(data.size() % 8 == 0);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // array buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size() * sizeof(float)), &data[0],
                 GL_STATIC_DRAW);

    // vertex attribute
    // attribute: 0-positon, 1-normal, 2-uv
    // params: index, size, type, normalized, stride, pointer
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLsizei(8 * sizeof(float)), (void *) nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, GLsizei(8 * sizeof(float)),
                          (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, GLsizei(8 * sizeof(float)),
                          (void *) (6 * sizeof(float)));

    // ebo
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    if (indices.empty())    // 如果没有提供 ebo，就自己造
    {
        size_t                    indices_cnt = data.size() / 8;
        std::vector<unsigned int> auto_indices;
        auto_indices.reserve(indices_cnt);
        for (int i = 0; i < indices_cnt; ++i)
            auto_indices.push_back(i);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) (indices_cnt * sizeof(unsigned int)),
                     auto_indices.data(), GL_STATIC_DRAW);
    } else
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) (indices.size() * sizeof(unsigned int)),
                     indices.data(), GL_STATIC_DRAW);

    // unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERROR();

    return VAO;
}


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
                     info.size, 0, info.external_format, info.external_type, info.data[i]);
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


GLuint shader_compile(const std::string &file_path, GLenum shader_type)
{
    // read file
    std::fstream      fs;
    std::stringstream ss;
    fs.open(file_path, std::ios::in);
    if (!fs.is_open())
    {
        SPDLOG_ERROR("fail to open file: {}", file_path);
        throw(std::exception());
    }
    for (std::string str; std::getline(fs, str); ss << str << '\n')
        ;
    fs.close();
    std::string shader_str   = ss.str();
    auto        shader_c_str = shader_str.c_str();

    // compile shader
    GLuint shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &shader_c_str, nullptr);
    glCompileShader(shader_id);

    // check
    int  success;
    char info[512];
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader_id, 512, nullptr, info);
        SPDLOG_ERROR("shader compile error: {}, info: \n{}", file_path, info);
        throw(std::exception());
    }

    CHECK_GL_ERROR();
    return shader_id;
}


GLuint shader_link(GLuint vertex, GLuint fragment, GLuint geometry)
{
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex);
    glAttachShader(program_id, fragment);
    if (geometry)
        glAttachShader(program_id, geometry);
    glLinkProgram(program_id);

    // check
    int  success;
    char info[512];
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program_id, 512, nullptr, info);
        SPDLOG_ERROR("shader link error, info: {}", info);
        throw(std::exception());
    }
    CHECK_GL_ERROR();
    return program_id;
}
