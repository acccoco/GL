#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/mesh.h"
#include "core/obj.h"
#include "core/texture.h"


struct Model {
    Mesh mesh;
    glm::vec3 pos{0, 0, 0};
    struct {
        GLuint id = 0;
        bool has = false;
    } tex_diffuse;

    glm::vec3 color_diffuse{};
    glm::vec3 color_ambient{};
    glm::vec3 color_specular{};

    [[nodiscard]] glm::mat4 model_matrix() const
    {
        return glm::translate(glm::one<glm::mat4>(), pos);
    }

    Model(const Mesh &mesh_, const glm::vec3 &pos_, GLuint diffuse_id)
        : mesh(mesh_),
          pos(pos_),
          tex_diffuse{diffuse_id, true}
    {}

    Model(const Mesh &mesh_, const glm::vec3 &pos_, const glm::vec3 kd_)
        : mesh(mesh_),
          pos(pos_),
          color_diffuse(kd_)
    {}

    explicit Model(const ObjData &data)
        : mesh(data.vertices, data.faces),
          color_diffuse(data.color_diffuse),
          color_ambient(data.color_ambient),
          color_specular(data.color_specular)
    {
        if (data.tex_diffuse_path.empty()) {
            tex_diffuse.has = false;
        } else {
            tex_diffuse.has = true;
            tex_diffuse.id = TextureManager::load_texture_(data.tex_diffuse_path);
        }
    }


    static std::vector<Model> load_obj(const std::string &file_path)
    {
        auto data_list = read_obj(file_path);

        std::vector<Model> models;
        models.reserve(data_list.size());
        for (auto &data: data_list) {
            models.emplace_back(data);
        }
        return models;
    }
};
