#version 330 core

/// 用于绘制线条的着色器

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_color;

out VS_FS {
    vec3 color;
} vs_fs;

uniform mat4 u_camera_mvp;


void main() {
    gl_Position = u_camera_mvp * vec4(in_pos, 1.0);

    vs_fs.color = in_color;
}