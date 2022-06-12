#include "../texture.h"
#include "../opengl-misc.h"


GLuint TextureManager::load_texture(const std::string &file_path, bool sRGB)
{
    SPDLOG_INFO("load texture: {}...", file_path);

    /// read file
    int width, height, channels;

    /// flip verticla 可以使得 data[0] 是图片的左下角，这样左下角就对应 texcoord 的 [0, 0]
    stbi_set_flip_vertically_on_load(true);
    auto data = stbi_load(file_path.c_str(), &width, &height, &channels, 0);
    if (!data)
        SPDLOG_ERROR("error on load texture.");


    /// create texture and bind
    Tex2DInfo info{
            .width  = width,
            .height = height,
            /// 不论图片是多少通道，OpenGL 内部格式都使用 RGBA，颜色通道以 0 填充，alpha 以 1 填充
            .internal_format = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8,
            .external_type   = GL_UNSIGNED_BYTE,
            .wrap_s          = GL_REPEAT,
            .wrap_t          = GL_REPEAT,
            .mipmap          = true,
            .data            = data,
    };
    switch (channels)
    {
        case 1: info.external_format = GL_RED; break;
        case 2: info.external_format = GL_RG; break;
        case 3: info.external_format = GL_RGB; break;
        case 4: info.external_format = GL_RGBA; break;
        default: LOG_AND_THROW("bad texture channes: {}", channels);
    }
    GLuint tex_id = new_tex2d(info);

    stbi_image_free(data);
    return tex_id;
}


GLuint load_cube_map(const CubeMapPath &tex_path, bool sRGB)
{
    SPDLOG_INFO("load cube map texture: {}", tex_path.pos_x);

    // cubemap 比较特殊，不需要竖直反转，所以 data[0] 是图片的左上角
    stbi_set_flip_vertically_on_load(false);

    // 用于存放 6 张纹理数据
    std::array<GLvoid *, 6> data{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    // 释放图片资源
    auto free_resource = [&data]() {
        for (GLvoid *d: data)
            stbi_image_free(d);    // 就是标准库的 free，free(nullptr) 不存在任何问题
    };

    std::array<std::string, 6> path_list = {
            tex_path.pos_x, tex_path.neg_x, tex_path.pos_y,
            tex_path.neg_y, tex_path.pos_z, tex_path.neg_z,
    };

    // 从文件中读取纹理
    int size_all, channels_all;
    for (int i = 0; i < 6; ++i)
    {
        int width, height, channels;
        data[i] = stbi_load(path_list[i].c_str(), &width, &height, &channels, 0);

        if (i == 0)
            size_all = width, channels_all = channels;

        /// 检查 cubemap 的各个面的尺寸/通道数是否相同
        if (width != height || width != size_all || channels != channels_all)
        {
            free_resource();
            LOG_AND_THROW("size/channels of cubemaps are not the same.");
        }
    }

    /// 检查通道数是否正常
    if (channels_all <= 0 || channels_all > 4)
    {
        free_resource();
        LOG_AND_THROW("cubemap channels error.");
    }

    static const std::vector<GLenum> channel_table = {0, GL_RED, GL_RG, GL_RGB, GL_RGBA};

    TexCubeInfo info = {
            .size            = size_all,
            .internal_format = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8,
            .external_format = channel_table[channels_all],
            .external_type   = GL_UNSIGNED_BYTE,
            .data            = data,
    };

    /// 创建纹理对象
    GLuint cube_map = new_cubemap(info);

    free_resource();
    return cube_map;
}
