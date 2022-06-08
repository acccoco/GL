#pragma once
#include <map>
#include <vector>

#include <glm/glm.hpp>
#include <tiny_gltf.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>


struct Material {
    std::string name;

    bool double_side{false};

    struct {
        double    metallic{0.f};
        double    roughness{0.8f};
        glm::vec4 base_color{0.6f};

        int tex_metallic_roughness{-1};
        int tex_base_color{-1};
    } metallic_roughness;

    // normal = normalize((<tex value> * 2.0 - 1.0) * vec3(<scale>, <scale>, 1.0))
    double normal_scale{1.0};
    int    tex_normal{-1};

    // occludedColor = lerp(color, color * <tex value>, <occlusion strength>)
    double occusion_strength{0.0};
    int    tex_occlusion{-1};

    glm::vec3 emissive{0.f};
    int       tex_emissive{-1};

    [[nodiscard]] bool has_tex_basecolor() const { return metallic_roughness.tex_base_color != -1; }
    [[nodiscard]] bool has_tex_metallic_roughness() const
    {
        return metallic_roughness.tex_metallic_roughness != -1;
    }
    [[nodiscard]] bool has_tex_occlusion() const { return tex_occlusion != -1; }
    [[nodiscard]] bool has_tex_normal() const { return tex_normal != -1; }
    [[nodiscard]] bool has_tex_emissive() const { return tex_emissive != -1; }
};


struct Mesh2 {
    GLuint      vao{};
    std::string name;
    GLint       primitive_mode{};    // TRIANGLE, LINE, POINT
    size_t      vertex_cnt{};
    int         index_component_type{};    // index 数据的类型，UNSIGNED_INT ...
    size_t index_offset{};    // 相对于 EBo 起始位置的偏移，可能多个 mesh 共享 ebo

    Material mat;
};


/// 渲染的一个对象，对应 gltf 中的 node
/// @note 目前仅支持 mesh
struct RTObj {
    glm::mat4 matrix{1.f};    // 表示位姿的矩阵

    Mesh2 mesh;
};


class ImportGLTF
{
public:
    /// 从文件中读取 gltf 模型
    explicit ImportGLTF(const std::string &filename)
    {
        tinygltf::TinyGLTF loader;
        std::string        err, warn;

        SPDLOG_INFO("load gltf file: {}", filename);

        /// 读取文件失败的处理
        if (!loader.LoadASCIIFromFile(&_gltf, &err, &warn, filename))
            SPDLOG_ERROR("fail to load gltf file. warn: {}, err: {}", warn, err);

        try
        {
            load_scene();
        } catch (std::exception &ex)
        {
            SPDLOG_ERROR("fail to load gltf sceen.");
        }
    }


    std::vector<RTObj> get_obj_list() { return _obj_list; }


private:
    /// node.mesh.primitive.attribute 可能取的值
    const struct {
        std::string pos     = "POSITION";
        std::string normal  = "NORMAL";
        std::string tex_0   = "TEXCOORD_0";    // VEC2
        std::string tangent = "TANGENT";       // VEC4
    } VERTEX_ATTRIBUTE_NAME;

    const struct {
        GLuint pos     = 0;
        GLuint normal  = 1;
        GLuint tex_0   = 2;
        GLuint tangent = 3;
    } VERTEX_ATTRBUTE_SLOT;


    /// gltf 的整个数据
    tinygltf::Model _gltf;


    /// 读取整个场景，包括摄像机信息，各个 mesh，以及 mesh 的层级
    /// @note 只支持第一个场景
    void load_scene();


    /// 处理 gltf 得到的结果
    std::vector<RTObj> _obj_list;


    /// 递归地处理 gltf 的 node，将结果写入 obj_list
    /// @TODO 支持摄像机和灯光
    /// @note 只支持 matrix 形式的位姿
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

    /// gltf 中用得到的 material
    std::map<int, Material> _mat_table;


    Material get_material(int mat_idx);


    /**
     * 创建一个 material
     * @note texture 只支持 TexCoord_0
     */
    Material create_material(int mat_idx);

#pragma endregion


#pragma region tex table

    /// gltf 中用得到的 texture
    std::map<int, GLuint> _tex_table;


    /// 根据 gltf 的 tex-idx 查找 texture id，如果找不到，就新建一个
    GLuint get_tex(int tex_idx);


    /**
     * 新建 texture
     * @note 支持 1-4 个通道的纹理
     * @note 每个通道只能是 8 位(ubyte) 或 16 位(ushort)
     */
    GLuint create_tex(int tex_idx);

#pragma endregion


#pragma region mesh table

    std::map<int, Mesh2> _mesh_table;


    /// 尝试从 _mesh_table 中读取模型
    Mesh2 get_mesh(int mesh_idx);


    /// 读取一个模型，只包含几何数据和 material，不包含位姿信息
    /// @note 只支持第一个 primitive，只支持 TexCoord0
    Mesh2 create_mesh(int mesh_idx);

#pragma endregion
};
