#pragma once

#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

/// 鼠标的位置\n
/// 通过 glfw 的回调函数来获得鼠标位置，因此用全局变量记录
static float g_mouse_dx = 0.f, g_mouse_dy = 0.f;

/// 记录鼠标按键的按下状态\n
/// 设定是摄像机视角控制 = 按下鼠标右键 + 移动鼠标\n
/// 然后两者的数据检测都是在 glfw 的回调中实现的，因此用一个全局变量来保存鼠标按键状态
static int g_mouse_right_button_state;

/// glfw 回调函数：鼠标位置移动
void mouse_pos_callback(GLFWwindow *, double xpos_in, double ypos_in);

/// glfw 回调函数：按下鼠标按键
void mouse_button_callback(GLFWwindow *, int button, int action, int);


class Camera
{
public:
    Camera() = default;
    explicit Camera(const glm::vec3 &pos)
        : position(pos)
    {}

    [[nodiscard]] glm::vec3 get_pos() const
    {
        return position;
    }

    [[nodiscard]] auto get_euler() const
    {
        return euler;
    }

    /// 基于欧拉角的旋转方式\n
    /// 更新欧拉角，用欧拉角更新 front 和 right 方向
    void update_dir_eular();

    /// 基于四元数的旋转\n
    /// 更新四元数，用四元数更新 front 和 right 方向
    void update_dir_quaternion();

    /// 通过 glfw 检测按键按下情况，用按键控制摄像机移动
    void update_position(GLFWwindow *window);

    /// view 矩阵
    [[nodiscard]] glm::mat4 view_matrix() const
    {
        return glm::lookAt(position, position + glm::normalize(front), UP);
    }

    /// 透视投影矩阵
    static glm::mat4 proj_matrix()
    {
        return glm::perspective(glm::radians(60.f), 1.f, 0.1f, 100.f);
    }

private:
    const glm::vec3 UP = {0.f, 1.f, 0.f};

    /// 摄像机的位置
    glm::vec3 position = {0.f, 5.f, 10.f};
    /// 摄像机的前方
    glm::vec3 front = {0.f, 0.f, -1.f};
    /// 摄像机的右方
    glm::vec3 right = {1.f, 0.f, 0.f};

    /// 欧拉角，用于表示摄像机的朝向
    struct {
        float yaw   = 0.f;    // 偏航
        float pitch = 0.f;    // 俯仰
    } euler;

    /// 四元数，用于表示摄像机的朝向
    glm::quat quat = {1.f, 0.f, 0.f, 0.f};

    const float CAMERA_ROTATE_SPEED = 0.05f;
    const float CAMERA_MOVE_SPEED   = 0.05f;
};

///===================================================================

inline void Camera::update_dir_eular()
{
    euler.yaw += g_mouse_dx * CAMERA_ROTATE_SPEED;
    euler.pitch += g_mouse_dy * CAMERA_ROTATE_SPEED;
    euler.yaw   = euler.yaw > 180.f ? euler.yaw - 360.f : euler.yaw;
    euler.yaw   = euler.yaw < -180.f ? euler.yaw + 360.f : euler.yaw;
    euler.pitch = std::min(std::max(euler.pitch, -89.f), 89.f);

    auto r_pitch = glm::radians(euler.pitch);
    auto r_yaw   = glm::radians(euler.yaw);

    front = {-cos(r_pitch) * sin(r_yaw), sin(r_pitch), -cos(r_pitch) * cos(r_yaw)};
    right = glm::normalize(glm::cross(front, UP));

    g_mouse_dx = 0, g_mouse_dy = 0;
}

inline void Camera::update_dir_quaternion()
{
    if (g_mouse_dx != 0)
    {
        quat = glm::normalize(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(g_mouse_dx) * CAMERA_ROTATE_SPEED, UP) *
                              quat);
    }
    if (g_mouse_dy != 0)
    {
        quat = glm::normalize(
                glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(g_mouse_dy) * CAMERA_ROTATE_SPEED, right) * quat);
    }

    front = glm::normalize(quat * glm::vec3{0, 0, -1} * glm::conjugate(quat));
    right = glm::normalize(glm::cross(front, UP));

    g_mouse_dx = 0, g_mouse_dy = 0;
}

inline void Camera::update_position(GLFWwindow *window)
{
    // parallel to the xz plane
    auto ahead = glm::normalize(glm::cross(UP, right));

    std::vector<std::pair<int, glm::vec3>> acc;
    std::vector<std::pair<int, glm::vec3>> m = {
            {GLFW_KEY_W, ahead}, {GLFW_KEY_S, -ahead}, {GLFW_KEY_A, -right},
            {GLFW_KEY_D, right}, {GLFW_KEY_Q, UP},     {GLFW_KEY_E, -UP},
    };
    for (auto &[key, dir]: m)
    {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            position += dir * CAMERA_MOVE_SPEED;
    }
}

inline void mouse_button_callback(GLFWwindow *, int button, int action, int)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        g_mouse_right_button_state = GLFW_PRESS;
    else
        g_mouse_right_button_state = GLFW_RELEASE;
}

inline void mouse_pos_callback(GLFWwindow *, double xpos_in, double ypos_in)
{
    static float last_x, last_y;
    if (g_mouse_right_button_state != GLFW_PRESS)
    {
        last_x = static_cast<float>(xpos_in);
        last_y = static_cast<float>(ypos_in);
        return;
    }
    g_mouse_dx = static_cast<float>(xpos_in) - last_x;
    g_mouse_dy = static_cast<float>(ypos_in) - last_y;
    last_x     = static_cast<float>(xpos_in);
    last_y     = static_cast<float>(ypos_in);
}
