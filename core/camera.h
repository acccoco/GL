#pragma once

#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

static const float CAMERA_ROTATE_SPEED = 0.05f;
static const float CAMERA_MOVE_SPEED = 0.05f;


static float mouse_dx = 0.f, mouse_dy = 0.f;


// pitch, yaw -->> front -->> view_matrix
class Camera
{
private:
    const glm::vec3 UP = {0.f, 1.f, 0.f};
    glm::vec3 position = {0.f, 5.f, 10.f};
    glm::vec3 front = {0.f, 0.f, -1.f};
    glm::vec3 right = {1.f, 0.f, 0.f};

    // default euler angle
    struct {
        float yaw = 0.f;  // 偏航
        float pitch = 0.f;// 俯仰
    } euler;

    // default quaternion
    glm::quat quat = {1.f, 0.f, 0.f, 0.f};

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

    // rotate by euler angle
    void rotate_euler()
    {
        euler.yaw += mouse_dx * CAMERA_ROTATE_SPEED;
        euler.pitch += mouse_dy * CAMERA_ROTATE_SPEED;
        euler.yaw = euler.yaw > 180.f ? euler.yaw - 360.f : euler.yaw;
        euler.yaw = euler.yaw < -180.f ? euler.yaw + 360.f : euler.yaw;
        euler.pitch = std::min(std::max(euler.pitch, -89.f), 89.f);

        auto r_pitch = glm::radians(euler.pitch);
        auto r_yaw = glm::radians(euler.yaw);
        front = {-cos(r_pitch) * sin(r_yaw), sin(r_pitch), -cos(r_pitch) * cos(r_yaw)};

        right = glm::normalize(glm::cross(front, UP));

        mouse_dx = 0, mouse_dy = 0;
    }

    // rotate by quaternion
    void rotate_quaternion()
    {
        if (mouse_dx != 0) {
            quat = glm::normalize(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(mouse_dx) * CAMERA_ROTATE_SPEED, UP) *
                                  quat);
        }
        if (mouse_dy != 0) {
            quat = glm::normalize(
                    glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(mouse_dy) * CAMERA_ROTATE_SPEED, right) * quat);
        }

        front = glm::normalize(quat * glm::vec3{0, 0, -1} * glm::conjugate(quat));
        right = glm::normalize(glm::cross(front, UP));

        mouse_dx = 0, mouse_dy = 0;
    }

    void move(GLFWwindow *window)
    {
        // parallel to the xz plane
        auto ahead = glm::normalize(glm::cross(UP, right));
        std::vector<std::pair<int, glm::vec3>> acc;
        std::vector<std::pair<int, glm::vec3>> m = {
                {GLFW_KEY_W, ahead}, {GLFW_KEY_S, -ahead}, {GLFW_KEY_A, -right},
                {GLFW_KEY_D, right}, {GLFW_KEY_Q, UP},     {GLFW_KEY_E, -UP},
        };
        for (auto &[key, dir]: m) {
            if (glfwGetKey(window, key) == GLFW_PRESS) position += dir * CAMERA_MOVE_SPEED;
        }
    }

    [[nodiscard]] glm::mat4 view_matrix() const
    {
        return glm::lookAt(position, position + glm::normalize(front), UP);
    }

    static glm::mat4 proj_matrix()
    {
        return glm::perspective(glm::radians(60.f), 1.f, 0.1f, 100.f);
    }
};

static int mouse_right_button_state;

static void mouse_pos_callback(GLFWwindow *, double xpos_in, double ypos_in)
{
    static float last_x, last_y;
    if (mouse_right_button_state != GLFW_PRESS) {
        last_x = static_cast<float>(xpos_in);
        last_y = static_cast<float>(ypos_in);
        return;
    }
    mouse_dx = static_cast<float>(xpos_in) - last_x;
    mouse_dy = static_cast<float>(ypos_in) - last_y;
    last_x = static_cast<float>(xpos_in);
    last_y = static_cast<float>(ypos_in);
}

static void mouse_button_callback(GLFWwindow *, int button, int action, int)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) mouse_right_button_state = GLFW_PRESS;
    else
        mouse_right_button_state = GLFW_RELEASE;
}
