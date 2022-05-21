#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

uniform mat4 u_model;
uniform mat4 u_camera_vp;

out VS_FS {
    vec3 world_pos;
    vec3 world_normal;
    vec2 uv;
} vs_fs;


void main()
{
    gl_Position = u_camera_vp * u_model * vec4(in_pos, 1.0);

    vs_fs.world_pos = (u_model * vec4(in_pos, 1.0)).xyz;
    vs_fs.world_normal = transpose(inverse(mat3(u_model))) * in_normal;
    vs_fs.uv = in_uv;
}