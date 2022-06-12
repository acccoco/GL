/**
 * 利用 Assimp 库读取 .obj 模型文件
 */
#pragma once

#include <vector>
#include <string>
#include <queue>

#include <spdlog/spdlog.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "./texture.h"
#include "./rt-object.h"


/**
 * 从 .obj 中读取出来的数据格式
 */
struct ObjData {
    std::vector<float>        vertices;
    std::vector<unsigned int> faces;
    std::string               tex_diffuse_path;
    glm::vec3                 color_diffuse{};
    glm::vec3                 color_ambient{};
    glm::vec3                 color_specular{};
};

/// 从指定 .obj 文件读取模型数据
std::vector<ObjData> read_obj(const std::string &file_path);

/// 从 assimp 的 mesh 中读取模型信息
ObjData process_mesh(const aiMesh &ai_mesh, const aiScene &scene, const std::string &dir_path);


/**
 * 读取 *.obj 模型文件，建立模型数据
 */
class ImportObj
{
public:
    explicit ImportObj(const std::string &filepath);

    /**
     * 读取 .obj 模型文件，返回 mesh 的列表
     */
    static std::vector<RTObject> load_obj(const std::string &filepath)
    {
        ImportObj im{filepath};
        return im._obj_list;
    }


private:
    /**
     * 递归地从 assimp 的 node 中提取模型
     */
    void process_node(const aiNode &node);


    /**
     * 读取 assimp 中的 mesh 中的几何数据，建立 VAO
     */
    static GLuint load_mesh_geometry(const aiMesh &mesh);


    /**
     * 读取模型的材质信息
     */
    Material load_material(const aiMaterial &ai_mat);


    /**
     * 从 assimp 的中读取几何和材质信息，创建 Mesh2 对象
     * @return
     */
    Mesh2 load_mesh(const aiMesh &mesh);


    std::vector<RTObject> _obj_list;    // 存放提取出的模型
    const aiScene        *_scene;       // .obj 模型对应的场景文件（Assimp)
    std::string           _dir_path;    // 模型所在目录
};
