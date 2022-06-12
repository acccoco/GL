#include "../import-obj.h"


std::vector<ObjData> read_obj(const std::string &file_path)
{
    SPDLOG_INFO("load obj: {}...", file_path);

    /// load file
    Assimp::Importer importer;
    const auto scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_GenNormals);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
        SPDLOG_WARN("fail to load model: {}", file_path);

    /// process model, 层序遍历
    auto                 dir_path = file_path.substr(0, file_path.find_last_of('/')) + '/';
    std::queue<aiNode *> nodes;
    nodes.push(scene->mRootNode);
    std::vector<ObjData> data_list;
    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        for (int i = 0; i < node->mNumMeshes; ++i)
        {
            data_list.push_back(process_mesh(*scene->mMeshes[node->mMeshes[i]], *scene, dir_path));
        }

        /// process children
        for (int i = 0; i < node->mNumChildren; ++i)
            nodes.push(node->mChildren[i]);
    }

    return data_list;
}


ObjData process_mesh(const aiMesh &ai_mesh, const aiScene &scene, const std::string &dir_path)
{
    ObjData data;

    // vertices
    {
        std::vector<float> vertices;
        vertices.reserve(ai_mesh.mNumVertices * 8);
        for (int j = 0; j < ai_mesh.mNumVertices; ++j)
        {
            auto pos    = ai_mesh.mVertices[j];
            auto normal = ai_mesh.mNormals[j];    // if (mesh->mNormals)
            auto uv     = ai_mesh.mTextureCoords[0] ? ai_mesh.mTextureCoords[0][j]
                                                    : aiVector3t<ai_real>(0, 0, 0);
            vertices.insert(vertices.end(), {pos.x, pos.y, pos.z});
            vertices.insert(vertices.end(), {normal.x, normal.y, normal.z});
            vertices.insert(vertices.end(), {uv.x, uv.y});
        }
        data.vertices = std::move(vertices);
    }

    // faces
    {
        std::vector<unsigned int> faces;
        for (int j = 0; j < ai_mesh.mNumFaces; ++j)
        {
            auto indices = ai_mesh.mFaces[j].mIndices;
            faces.insert(faces.end(), {indices[0], indices[1], indices[2]});
        }
        data.faces = std::move(faces);
    }

    auto material = scene.mMaterials[ai_mesh.mMaterialIndex];

    // diffuse texture
    aiString tex_file_name;
    material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_file_name);
    data.tex_diffuse_path = tex_file_name.length == 0 ? "" : dir_path + tex_file_name.C_Str();

    // color
    aiColor3D temp_color;
    material->Get(AI_MATKEY_COLOR_DIFFUSE, temp_color);
    data.color_diffuse = {temp_color[0], temp_color[1], temp_color[2]};
    material->Get(AI_MATKEY_COLOR_AMBIENT, temp_color);
    data.color_ambient = {temp_color[0], temp_color[1], temp_color[2]};
    material->Get(AI_MATKEY_COLOR_SPECULAR, temp_color);
    data.color_specular = {temp_color[0], temp_color[1], temp_color[2]};

    return data;
}


ImportObj::ImportObj(const std::string &filepath)
{
    Assimp::Importer impoter;

    // 模型三角化，自动生成法向量，还可以选择生成 Tangent
    _scene = impoter.ReadFile(filepath, aiProcess_Triangulate | aiProcess_GenNormals);
    if (!_scene || (_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !_scene->mRootNode)
        LOG_AND_THROW("fail to load model: {}", filepath);

    // 模型所在文件夹的路径 TODO 使用 filesystem
    _dir_path = filepath.substr(0, filepath.find_last_of('/')) + '/';

    process_node(*_scene->mRootNode);
}


GLuint ImportObj::load_mesh_geometry(const aiMesh &mesh)

{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const size_t vertex_cnt = mesh.mNumVertices;

    // 提取模型的 position 和 normal 数据
    std::vector<float> positon, normal;
    positon.reserve(3 * vertex_cnt);
    normal.reserve(3 * vertex_cnt);
    const size_t position_data_byte = sizeof(float) * vertex_cnt * 3;    // pos 数据大小
    const size_t normal_data_byte   = sizeof(float) * vertex_cnt * 3;    // normal 数据大小
    for (int i = 0; i < vertex_cnt; ++i)
    {
        combine(positon, {mesh.mVertices[i].x, mesh.mVertices[i].y, mesh.mVertices[i].z});
        combine(normal, {mesh.mNormals[i].x, mesh.mNormals[i].y, mesh.mNormals[i].z});
    }

    // 提取模型的 texcoord 数据
    // 模型可能有 0 到多组纹理数据，需要判断一下是否有纹理数据
    const bool         has_texcoord       = mesh.HasTextureCoords(0);
    const size_t       texcoord_data_byte = sizeof(float) * vertex_cnt * 2;
    std::vector<float> texcoord;
    if (has_texcoord)
    {
        texcoord.reserve(2 * vertex_cnt);
        for (int i = 0; i < vertex_cnt; ++i)
            combine(texcoord, {mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y});
    } else
        SPDLOG_INFO("mesh has no texcoord.");

    // 创建 VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr) (sizeof(float) * vertex_cnt * (has_texcoord ? 8 : 6)), nullptr,
                 GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr) position_data_byte, positon.data());
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr) position_data_byte, (GLsizeiptr) normal_data_byte,
                    normal.data());
    if (has_texcoord)
        glBufferSubData(GL_ARRAY_BUFFER, (GLintptr) (position_data_byte + normal_data_byte),
                        (GLsizeiptr) texcoord_data_byte, texcoord.data());

    // 指定顶点属性
    glEnableVertexAttribArray(VERTEX_ATTRBUTE_SLOT.pos);
    glVertexAttribPointer(VERTEX_ATTRBUTE_SLOT.pos, 3, GL_FLOAT, GL_FALSE,
                          (GLsizei) (3 * sizeof(float)), nullptr);
    glEnableVertexAttribArray(VERTEX_ATTRBUTE_SLOT.normal);
    glVertexAttribPointer(VERTEX_ATTRBUTE_SLOT.normal, 3, GL_FLOAT, GL_FALSE,
                          (GLsizei) (3 * sizeof(float)), (void *) position_data_byte);
    if (has_texcoord)
    {
        glEnableVertexAttribArray(VERTEX_ATTRBUTE_SLOT.tex_0);
        glVertexAttribPointer(VERTEX_ATTRBUTE_SLOT.tex_0, 2, GL_FLOAT, GL_FALSE,
                              (GLsizei) (2 * sizeof(float)),
                              (void *) (position_data_byte + normal_data_byte));
    }

    // EBO
    size_t                    face_cnt = mesh.mNumFaces;
    std::vector<unsigned int> indices;
    indices.reserve(3 * face_cnt);
    for (int i = 0; i < face_cnt; ++i)
        // 前面已经指定要对面进行三角化，因此这里可以直接读取 mIndices[0, 1, 2]
        combine(indices, {mesh.mFaces[i].mIndices[0], mesh.mFaces[i].mIndices[1],
                          mesh.mFaces[i].mIndices[2]});
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) (3 * face_cnt * sizeof(unsigned int)),
                 indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    CHECK_GL_ERROR();
    return vao;
}


Material ImportObj::load_material(const aiMaterial &ai_mat)
{
    Material mat;

    /// diffuse texture
    aiString diffuse_tex_filename;
    ai_mat.GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_tex_filename);
    if (diffuse_tex_filename.length != 0)
        mat.metallic_roughness.tex_base_color =
                (int) TextureManager::load_texture_(_dir_path + diffuse_tex_filename.C_Str());

    /// color
    aiColor3D temp_color;
    ai_mat.Get(AI_MATKEY_COLOR_DIFFUSE, temp_color);
    mat.metallic_roughness.base_color = {temp_color[0], temp_color[1], temp_color[2], 1.f};

    return mat;
}


Mesh2 ImportObj::load_mesh(const aiMesh &mesh)
{
    return Mesh2{
            .vao            = load_mesh_geometry(mesh),
            .primitive_mode = GL_TRIANGLES,
            // TODO 暂时只能创建三角形
            .index_cnt            = mesh.mNumFaces * 3,
            .index_component_type = GL_UNSIGNED_INT,
            .index_offset         = 0,
            .mat                  = load_material(*_scene->mMaterials[mesh.mMaterialIndex]),
    };
}


void ImportObj::process_node(const aiNode &node)
{
    // 将当前节点的模型加入到 obj list 中
    for (int i = 0; i < node.mNumMeshes; ++i)
        _obj_list.emplace_back(load_mesh(*_scene->mMeshes[node.mMeshes[i]]));


    // 递归地处理子节点
    for (int i = 0; i < node.mNumChildren; ++i)
        process_node(*node.mChildren[i]);
}
