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


/**
 * 从 .obj 中读取出来的数据格式
 */
struct ObjData {
    std::vector<float> vertices;
    std::vector<unsigned int> faces;
    std::string tex_diffuse_path;
    glm::vec3 color_diffuse{};
    glm::vec3 color_ambient{};
    glm::vec3 color_specular{};
};


static ObjData process_mesh(const aiMesh &ai_mesh, const aiScene &scene, const std::string &dir_path);

std::vector<ObjData> read_obj(const std::string &file_path)
{
    SPDLOG_INFO("load obj: {}...", file_path);

    /// load file
    Assimp::Importer importer;
    const auto scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_GenNormals);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
        SPDLOG_WARN("fail to load model: {}", file_path);

    /// process model, 层序遍历
    auto dir_path = file_path.substr(0, file_path.find_last_of('/')) + '/';
    std::queue<aiNode *> nodes;
    nodes.push(scene->mRootNode);
    std::vector<ObjData> data_list;
    while (!nodes.empty()) {
        auto node = nodes.front();
        nodes.pop();

        for (int i = 0; i < node->mNumMeshes; ++i) {
            data_list.push_back(process_mesh(*scene->mMeshes[node->mMeshes[i]], *scene, dir_path));
        }

        /// process children
        for (int i = 0; i < node->mNumChildren; ++i)
            nodes.push(node->mChildren[i]);
    }

    return data_list;
}


static ObjData process_mesh(const aiMesh &ai_mesh, const aiScene &scene, const std::string &dir_path)
{
    ObjData data;

    // vertices
    {
        std::vector<float> vertices;
        vertices.reserve(ai_mesh.mNumVertices * 8);
        for (int j = 0; j < ai_mesh.mNumVertices; ++j) {
            auto pos = ai_mesh.mVertices[j];
            auto normal = ai_mesh.mNormals[j];// if (mesh->mNormals)
            auto uv = ai_mesh.mTextureCoords[0] ? ai_mesh.mTextureCoords[0][j] : aiVector3t<ai_real>(0, 0, 0);
            vertices.insert(vertices.end(), {pos.x, pos.y, pos.z});
            vertices.insert(vertices.end(), {normal.x, normal.y, normal.z});
            vertices.insert(vertices.end(), {uv.x, uv.y});
        }
        data.vertices = std::move(vertices);
    }

    // faces
    {
        std::vector<unsigned int> faces;
        for (int j = 0; j < ai_mesh.mNumFaces; ++j) {
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
