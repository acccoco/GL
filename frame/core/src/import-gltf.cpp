#include "../import-gltf.h"
#include "../misc.h"
#include "../texture.h"


ImportGLTF::ImportGLTF(const std::string &filename)
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


void ImportGLTF::load_scene()
{
    /// 只处理 scene 数量为 1 的情况
    if (_gltf.scenes.size() != 1)
        SPDLOG_INFO("currently, only one scene is supported.");
    if (_gltf.scenes.empty())
        LOG_AND_THROW("no scene in gltf.");
    tinygltf::Scene scene = _gltf.scenes[0];

    for (size_t node_idx: scene.nodes)
    {
        load_node(_gltf.nodes[node_idx], glm::mat4(1.f));
    }
}


void ImportGLTF::load_node(const tinygltf::Node &node, const glm::mat4 &parent_matrix_world)
{
    /// 计算当前 node 的位姿
    glm::mat4 cur_matrix_world;
    {
        glm::mat4 cur_matrix = glm::mat4(1.f);    // 当前 node 相对于父 node 的位姿
        if (!node.matrix.empty())
            cur_matrix =
                    glm::mat4(node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
                              node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
                              node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
                              node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]);
        cur_matrix_world = parent_matrix_world * cur_matrix;
    }

    /// 如果当前节点是 mesh
    if (node.mesh >= 0)
    {
        if (node.mesh >= _gltf.meshes.size())
            LOG_AND_THROW("current node is mesh, but idx out of range: {}", node.mesh);
        
        _obj_list.emplace_back(get_mesh(node.mesh), cur_matrix_world);
    }

    /// 子节点
    for (int child: node.children)
    {
        load_node(_gltf.nodes[child], cur_matrix_world);
    }
}


GLuint ImportGLTF::get_ebo_vbo(int buffer_view_idx)
{
    /// 可以找到 buffer view 对应的 ebo/vbo
    auto res = _bufferview_bo_table.find(buffer_view_idx);
    if (res != _bufferview_bo_table.end())
        return res->second;

    /// 检查 buffer view 是否符合 ebo/vbo 的条件
    const tinygltf::BufferView &buffer_view = _gltf.bufferViews[buffer_view_idx];
    const tinygltf::Buffer     &buffer      = _gltf.buffers[buffer_view.buffer];
    if (buffer_view.target != TINYGLTF_TARGET_ARRAY_BUFFER &&
        buffer_view.target != TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER)
        LOG_AND_THROW("buffer view type is not [ARRAY_BUFFER] or [ELEMENT_ARRAY_BUFFER]");

    /// 新建 ebo/vbo
    GLuint ebo_vbo;
    glGenBuffers(1, &ebo_vbo);
    glBindBuffer(buffer_view.target, ebo_vbo);
    glBufferData(buffer_view.target, (GLsizeiptr) buffer_view.byteLength,
                 buffer.data.data() + buffer_view.byteOffset, GL_STATIC_DRAW);
    glBindBuffer(buffer_view.target, 0);

    /// 返回
    _bufferview_bo_table[buffer_view_idx] = ebo_vbo;
    return ebo_vbo;
}


Material ImportGLTF::get_material(int mat_idx)
{
    /// 尝试在 _mat_table 中查找 material
    {
        auto mat_iter = _mat_table.find(mat_idx);
        if (mat_iter != _mat_table.end())
            return mat_iter->second;
    }

    /// 判断 material_idx 是否有效
    if (mat_idx < 0 || mat_idx >= _gltf.materials.size())
        LOG_AND_THROW("material idx out of range: {}", mat_idx);

    /// 新建 material
    Material mat        = create_material(mat_idx);
    _mat_table[mat_idx] = mat;

    return mat;
}


Material ImportGLTF::create_material(int mat_idx)
{
    const tinygltf::Material &gltf_mat = _gltf.materials[mat_idx];

    Material mat = {
            .name        = gltf_mat.name,
            .double_side = gltf_mat.doubleSided,
    };

    /// 不支持 0 之外的 TexCoord
#define CHECK_TEXCOORD(tex)                                                                        \
    if (tex.texCoord != 0)                                                                         \
    LOG_AND_THROW("unsupported texcoord: {}", tex.texCoord)

    /// matallic roughness
    {
        const tinygltf::PbrMetallicRoughness &pbr = gltf_mat.pbrMetallicRoughness;

        mat.metallic_roughness.metallic   = pbr.metallicFactor;
        mat.metallic_roughness.roughness  = pbr.roughnessFactor;
        mat.metallic_roughness.base_color = {pbr.baseColorFactor[0], pbr.baseColorFactor[1],
                                             pbr.baseColorFactor[2], pbr.baseColorFactor[3]};
        CHECK_TEXCOORD(pbr.baseColorTexture);
        mat.metallic_roughness.tex_base_color =
                (pbr.baseColorTexture.index == -1) ? -1 : (int) get_tex(pbr.baseColorTexture.index);
        CHECK_TEXCOORD(pbr.metallicRoughnessTexture);
        mat.metallic_roughness.tex_metallic_roughness =
                (pbr.metallicRoughnessTexture.index == -1)
                        ? -1
                        : (int) get_tex(pbr.metallicRoughnessTexture.index);
    }

    /// normal texture
    {
        mat.normal_scale = gltf_mat.normalTexture.scale;
        CHECK_TEXCOORD(gltf_mat.normalTexture);
        mat.tex_normal = (gltf_mat.normalTexture.index == -1)
                                 ? -1
                                 : (int) get_tex(gltf_mat.normalTexture.index);
    }

    /// occlusion texture
    {
        mat.occusion_strength = gltf_mat.occlusionTexture.strength;
        CHECK_TEXCOORD(gltf_mat.occlusionTexture);
        mat.tex_occlusion = (gltf_mat.occlusionTexture.index == -1)
                                    ? -1
                                    : (int) get_tex(gltf_mat.occlusionTexture.index);
    }

    /// emissive
    {
        mat.emissive = {
                gltf_mat.emissiveFactor[0],
                gltf_mat.emissiveFactor[1],
                gltf_mat.emissiveFactor[2],
        };
        CHECK_TEXCOORD(gltf_mat.emissiveTexture);
        mat.tex_emissive = (gltf_mat.emissiveTexture.index == -1)
                                   ? -1
                                   : (int) get_tex(gltf_mat.emissiveTexture.index);
    }

#undef CHECK_TEXCOORD
    return mat;
}


GLuint ImportGLTF::get_tex(int tex_idx)
{
    /// 可以找到
    {
        auto iter = _tex_table.find(tex_idx);
        if (iter != _tex_table.end())
            return iter->second;
    }

    /// 检查 tex idx 是否有效
    if (tex_idx < 0 || tex_idx >= _gltf.textures.size())
        LOG_AND_THROW("tex idx out of range: {}", tex_idx);

    /// 将新建的 texture 存起来
    GLuint tex_id       = create_tex(tex_idx);
    _tex_table[tex_idx] = tex_id;

    return tex_id;
}


GLuint ImportGLTF::create_tex(int tex_idx)

{
    const tinygltf::Texture &tex     = _gltf.textures[tex_idx];
    const tinygltf::Image   &image   = _gltf.images[tex.source];
    const tinygltf::Sampler &sampler = _gltf.samplers[tex.sampler];

    if (image.component < 0 || image.component > 4)    // 检查图像的颜色通道数
        LOG_AND_THROW("image color channels error: {}", image.component);

    /// 像素颜色通道数和 external-format 的对应关系
    const static std::array<GLenum, 4> external_format_table = {
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
    };

    /// 每个通道 8 位，unsigned normalized 的 internal format
    const static std::array<GLint, 4> internal_format_8N_table = {
            GL_R8,
            GL_RG8,
            GL_RGB8,
            GL_RGBA8,
    };

    /// 每个通道 16 位，unsigned normalized 的 internal format
    const static std::array<GLint, 4> internal_format_16N_table = {
            GL_R16,
            GL_RG16,
            GL_RGB16,
            GL_RGBA16,
    };

    GLenum external_format = external_format_table[image.component - 1];
    GLenum external_type   = image.pixel_type;

    GLint internal_format;
    if (external_type == GL_UNSIGNED_BYTE)    // 每个通道 8 bits
        internal_format = internal_format_8N_table[image.component - 1];
    else if (external_type == GL_UNSIGNED_INT)    // 每个通道 16 bits
        internal_format = internal_format_16N_table[image.component - 1];
    else
        LOG_AND_THROW("unsupported image external type: {}", external_type);

    Tex2DInfo info = {
            .width           = image.width,
            .height          = image.height,
            .internal_format = internal_format,
            .external_format = external_format,
            .external_type   = external_type,
            .wrap_s          = sampler.wrapS,
            .wrap_t          = sampler.wrapT,
            .data            = image.image.data(),
    };
    info.filter_min = (sampler.minFilter == -1) ? GL_LINEAR : sampler.minFilter;
    info.filter_mag = (sampler.magFilter == -1) ? GL_LINEAR : sampler.magFilter;

    if (is_one_of(info.filter_min, {GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST,
                                    GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR}))
        info.mipmap = true;

    return new_tex2d(info);
}


Mesh2 ImportGLTF::get_mesh(int mesh_idx)
{
    /// 在 _mesh_table 中寻找
    {
        auto mesh_iter = _mesh_table.find(mesh_idx);
        if (mesh_iter != _mesh_table.end())
            return mesh_iter->second;
    }

    Mesh2 mesh            = create_mesh(mesh_idx);
    _mesh_table[mesh_idx] = mesh;

    return mesh;
}


Mesh2 ImportGLTF::create_mesh(int mesh_idx)

{
    const tinygltf::Mesh &gltf_mesh = _gltf.meshes[mesh_idx];

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /// 只考虑 mesh 的第一个 primitive
    if (gltf_mesh.primitives.empty())
        LOG_AND_THROW("mesh has no primitive.");
    tinygltf::Primitive primitive = gltf_mesh.primitives[0];

    /// EBO
    const tinygltf ::Accessor &index_accessor = _gltf.accessors[primitive.indices];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, get_ebo_vbo(index_accessor.bufferView));

    /// 顶点属性
    for (const auto &attr: primitive.attributes)
    {
        tinygltf::Accessor accessor = _gltf.accessors[attr.second];
        /// 如果 buffer view 的 byte stride == 0，下面的函数就可以计算出 byte_stride 的大小
        int byte_stride = accessor.ByteStride(_gltf.bufferViews[accessor.bufferView]);

        /// 顶点属性里面有几个分量，例如 position 是 vec3，所以 size = 3
        int size = (accessor.type == TINYGLTF_TYPE_SCALAR) ? 1 : accessor.type;

        GLuint vertex_attr_idx;
        {
            if (attr.first == VERTEX_ATTRIBUTE_NAME.pos)
                vertex_attr_idx = VERTEX_ATTRBUTE_SLOT.pos;
            else if (attr.first == VERTEX_ATTRIBUTE_NAME.normal)
                vertex_attr_idx = VERTEX_ATTRBUTE_SLOT.normal;
            else if (attr.first == VERTEX_ATTRIBUTE_NAME.tex_0)
                vertex_attr_idx = VERTEX_ATTRBUTE_SLOT.tex_0;
            else if (attr.first == VERTEX_ATTRIBUTE_NAME.tangent)
                vertex_attr_idx = VERTEX_ATTRBUTE_SLOT.tangent;
            else
            {
                SPDLOG_WARN("gltf mesh[{}] has unsupported vertex attribute: {}.", mesh_idx,
                            attr.first);
                continue;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, get_ebo_vbo(accessor.bufferView));
        glEnableVertexAttribArray(vertex_attr_idx);
        glVertexAttribPointer(vertex_attr_idx, size, accessor.componentType,
                              accessor.normalized ? GL_TRUE : GL_FALSE, byte_stride,
                              (void *) accessor.byteOffset);
    }

    // 材质
    Material mat = get_material(primitive.material);

    return Mesh2{
            .vao                  = vao,
            .name                 = gltf_mesh.name,
            .primitive_mode       = primitive.mode,
            .index_cnt            = index_accessor.count,
            .index_component_type = index_accessor.componentType,
            .index_offset         = index_accessor.byteOffset,
            .mat                  = mat,
    };
}
