#pragma once

#include <utility>

#include "./mesh.h"
#include "./light.h"


struct RTObjectBase {
    explicit RTObjectBase(const glm::mat4 &matrix_ = glm::mat4(1.f)) { set_matrix(matrix_); }

    /**
     * 改变模型的位置
     */
    void set_pos(const glm::vec3 &pos);

    [[nodiscard]] glm::mat4 matrix() const { return _matrix; }
    [[nodiscard]] glm::vec3 position() const { return _position; }

    /**
     * 改变模型的位姿，更新 position，rotate，scale
     * @note 假设矩阵只包含 scale, translate, rotate 信息，且 scale 是正数（非零），M = TRS
     */
    void set_matrix(const glm::mat4 &matrix);

private:
    /**
     * 更新了 positon，rotate，scale 后，用这些值来更新 matrix
     */
    void update_matrix();

    glm::vec3 _position{};
    glm::vec3 _scale{1.f};
    // quaternion = <\cos\theta/2, u\sin\theta/2>，其中 u 是转轴(vec3)，\theta 是旋转角度
    glm::quat _rotate{1.f, glm::vec3{0.f}};
    glm::mat4 _matrix{1.f};    // 表示位姿的矩阵
};


/**
 * 渲染的一个对象，对应 gltf 中的 node
 */
struct RTObject : public RTObjectBase {

    explicit RTObject(Mesh2 mesh_, const glm::mat4 &matrix_ = glm::mat4(1.f))
        : RTObjectBase(matrix_), mesh(std::move(mesh_))
    {}


    Mesh2 mesh;
};