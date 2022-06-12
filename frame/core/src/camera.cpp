#include "../camera.h"


glm::mat4 Camera2::view_matrix() const
{
    return glm::lookAt(_position, _position + glm::normalize(_front_cache), UP);
}


void Camera2::tick_rotate()
{
    std::array<double, 2> cursor_pos = Window::cursor_pos();
    if (Window::mouse_button_last_action(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        rotate_eular(cursor_pos[0] - _cursor_pre_posx, cursor_pos[1] - _cursor_pre_posy);
    _cursor_pre_posx = cursor_pos[0];
    _cursor_pre_posy = cursor_pos[1];
}


void Camera2::tick_move()
{
    auto ahead = glm::normalize(glm::cross(UP, _right_cache));

    /**
         * 键盘按键与方向控制的对应关系
         */
    std::vector<std::pair<int, glm::vec3>> m = {
            {GLFW_KEY_W, ahead},        {GLFW_KEY_S, -ahead}, {GLFW_KEY_A, -_right_cache},
            {GLFW_KEY_D, _right_cache}, {GLFW_KEY_Q, UP},     {GLFW_KEY_E, -UP},
    };
    for (auto &[key, dir]: m)

        if (Window::keyboard_last_action(key) == GLFW_PRESS)
            _position += dir * CAMERA_MOVE_SPEED;
}


void Camera2::rotate_eular(double delta_x, double delta_y)
{
    euler.yaw += (float) delta_x * CAMERA_ROTATE_SPEED;
    euler.pitch += (float) delta_y * CAMERA_ROTATE_SPEED;

    // 限制欧拉角的角度，防止死锁，简化显示
    euler.yaw   = euler.yaw > 180.f ? euler.yaw - 360.f : euler.yaw;
    euler.yaw   = euler.yaw < -180.f ? euler.yaw + 360.f : euler.yaw;
    euler.pitch = std::min(std::max(euler.pitch, -89.f), 89.f);

    auto r_pitch = glm::radians(euler.pitch);
    auto r_yaw   = glm::radians(euler.yaw);

    _front_cache = {-cos(r_pitch) * sin(r_yaw), sin(r_pitch), -cos(r_pitch) * cos(r_yaw)};
    _right_cache = glm::normalize(glm::cross(_front_cache, UP));
}