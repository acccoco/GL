#version 330 core

/// 显示灰模的着色器

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;

out VS_FS {
    vec3 normal;
    vec3 pos_world;
} vs_fs;

uniform mat4 u_model;
uniform mat4 u_vp;


void main() {
    vec4 temp = u_model * vec4(in_pos, 1.0);
    gl_Position = u_vp * temp;

    vs_fs.normal = in_normal;
    vs_fs.pos_world = temp.xyz / temp.w;
}