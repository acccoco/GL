#pragma once

#include <map>
#include <string>

#include <stb_image.h>

#include "./misc.h"


/**
 * @brief 纹理资源管理
 * 如果多个模型引用同一份纹理，这个类可以避免重复读取文件。
 * 通过静态 map 实现资源管理
 */
class TextureManager
{
public:
    static GLuint load_texture_(const std::string &file_path)
    {
        if (m.find(file_path) == m.end())
            m[file_path] = load_texture(file_path);
        return m[file_path];
    }

private:
    TextureManager() = default;
    inline static std::map<std::string, GLuint> m;

    /**
     * 从文件中读取纹理，创建 OpenGL 的纹理对象
     * @param sRGB 是否是 sRGB 的颜色空间，default = false
     * @note 默认只支持每通道 8 bits 的图片
     */
    static GLuint load_texture(const std::string &file_path, bool sRGB = false);
};


/// cubemap 各个面的路径
struct CubeMapPath {
    std::string pos_x;
    std::string neg_x;
    std::string pos_y;
    std::string neg_y;
    std::string pos_z;
    std::string neg_z;
};
/**
 * 从文件中读取 cubemap
 * @note 默认颜色通道是 8 位的
 * @note OpenGL 内部使用的格式是 GL_RGBA8（还有考虑 sRGB)
 */
GLuint load_cube_map(const CubeMapPath &tex_path, bool sRGB = false);
