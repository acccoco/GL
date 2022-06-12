#pragma once

#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "./window.h"


struct Camera2 {

    const float fov    = 60.f;
    const float near   = 0.1f;     // 近平面的距离
    const float far    = 100.f;    // 远平面的距离
    const float aspect = 1.0f;     // 近平面的长款比

    Camera2() : _proj_matrix(glm::perspective(glm::radians(fov), aspect, near, far)) {}


    [[nodiscard]] glm::vec3 get_pos() const { return _position; }
    [[nodiscard]] glm::vec3 get_right() const { return _right_cache; }
    [[nodiscard]] glm::vec3 get_front() const { return _front_cache; }
    [[nodiscard]] auto      get_euler() const { return euler; }


    /**
     * 摄像机的 view matrix
     */
    [[nodiscard]] glm::mat4 view_matrix() const;

    /**
     * 摄像机的 projet matrix
     */
    [[nodiscard]] glm::mat4 proj_matrix() const { return _proj_matrix; }

    /**
     * 摄像机旋转：按下鼠标右键才能旋转摄像机
     */
    void tick_rotate();

    /**
     * 摄像机移动位置，会改变 _position
     */
    void tick_move();

private:
    double _cursor_pre_posx{}, _cursor_pre_posy{};    // 鼠标之前的位置

    glm::vec3 _position    = {0.f, 5.f, 10.f};    // 摄像机的位置
    glm::vec3 _front_cache = {0.f, 0.f, -1.f};    // 摄像机的前方，根据欧拉角计算出
    glm::vec3 _right_cache = {1.f, 0.f, 0.f};     // 摄像机的右方，根据欧拉角计算出

    struct {
        float yaw   = 0.f;    // 偏航
        float pitch = 0.f;    // 俯仰
    } euler;                  // 欧拉角，用于表示摄像机的朝向

    const glm::mat4 _proj_matrix;

    const glm::vec3 UP = {0.f, 1.f, 0.f};    // 世界的 up 方向

    const float CAMERA_ROTATE_SPEED = 0.05f;
    const float CAMERA_MOVE_SPEED   = 0.05f;

    /**
     * 通过欧拉角来旋转摄像机，会改变 eular，并更新 front 和 right
     */
    void rotate_eular(double delta_x, double delta_y);
};
