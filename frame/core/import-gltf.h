#pragma once
#include <map>
#include <vector>

#include <glm/glm.hpp>
#include <tiny_gltf.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include "./mesh.h"
#include "./material.h"
#include "./rt-object.h"


class ImportGLTF
{
public:
    /**
     * 从文件中读取 gltf 模型
     */
    explicit ImportGLTF(const std::string &filename);


    /**
     * 获取从文件中读取到的 object
     */
    std::vector<RTObject> get_obj_list() { return _obj_list; }


    /**
     * 读取文件，获取模型
     */
    static std::vector<RTObject> load_gltf(const std::string &filepath)
    {
        ImportGLTF im{filepath};
        return im.get_obj_list();
    }


private:
    tinygltf::Model       _gltf;        // gltf 的整个数据
    std::vector<RTObject> _obj_list;    // 从 gltf 中读到的 object

    /**
     * node.mesh.primitive.attribute 可能取的值
     */
    const struct {
        std::string pos     = "POSITION";
        std::string normal  = "NORMAL";
        std::string tex_0   = "TEXCOORD_0";    // VEC2
        std::string tangent = "TANGENT";       // VEC4
    } VERTEX_ATTRIBUTE_NAME;

    /**
     * 读取整个场景，包括摄像机信息，各个 mesh，以及 mesh 的层级
     * @note 只支持第一个场景
     */
    void load_scene();

    /**
     * 递归地处理 gltf 的 node，将结果写入 obj_list
     * @param node 要处理的 gltf 节点
     * @param parent_matrix_world 父节点在世界坐标系下的位姿
     * @TODO 支持摄像机和灯光
     * @note 只支持 matrix 形式的位姿
     */
    void load_node(const tinygltf::Node &node, const glm::mat4 &parent_matrix_world);

#pragma region buffer view

    /// 根据 buffer view 创建对应的 vbo 或者 ebo，存放在这个表中
    std::map<int, GLuint> _bufferview_bo_table;


    /**
     * 根据 gltf 中 buffer-view 的 index，找到对应的 VBO 或 EBO
     * @note 只支持 ARRAY_BUFFER 和 ELEMENT_ARRAY_BUFFER
     */
    GLuint get_ebo_vbo(int buffer_view_idx);

#pragma endregion


#pragma region material table

    std::map<int, Material> _mat_table;    // gltf 中用得到的 material


    Material get_material(int mat_idx);


    /**
     * 创建一个 material
     * @param mat_idx gltf 文件中的 material index
     * @note texture 只支持 TexCoord_0
     */
    Material create_material(int mat_idx);

#pragma endregion


#pragma region tex table

    std::map<int, GLuint> _tex_table;    // gltf 中用得到的 texture


    /**
     * 根据 gltf 的 tex-idx 查找 texture id，如果找不到，就新建一个
     * @param tex_idx gltf 文件中的 textrue index
     * @return OpenGL 中的 texture id
     */
    GLuint get_tex(int tex_idx);


    /**
     * 新建 texture
     * @param tex_idx gltf 文件中的 textrue index
     * @note 支持 1-4 个通道的纹理
     * @note 每个通道只能是 8 位(ubyte) 或 16 位(ushort)
     */
    GLuint create_tex(int tex_idx);

#pragma endregion


#pragma region mesh table

    std::map<int, Mesh2> _mesh_table;


    /**
     * 尝试从 _mesh_table 中读取模型
     * @param mesh_idx gltf 文件中的 mesh index
     */
    Mesh2 get_mesh(int mesh_idx);


    /**
     * 读取一个模型，只包含几何数据和 material，不包含位姿信息
     * @param mesh_idx gltf 文件中的 mesh index
     * @note 只支持第一个 primitive，只支持 TexCoord0
     */
    Mesh2 create_mesh(int mesh_idx);

#pragma endregion
};
