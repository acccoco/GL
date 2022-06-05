#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out VS_FS {
    vec3 pos_view;
    vec3 normal_view;
    vec2 uv;
} vs_fs;


void main()
{
    vec4 pos = vec4(in_pos, 1.0);
    pos = u_view * u_model * pos;
    vs_fs.pos_view = pos.xyz / pos.w;

    gl_Position = u_proj * pos;
    vs_fs.normal_view  = transpose(inverse(mat3(u_view * u_model))) * in_normal;
    vs_fs.uv = in_uv;
}