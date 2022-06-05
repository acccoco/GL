#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;

out VS_FS {
    vec2 uv;
} vs_fs;




void main() {
    gl_Position = vec4(in_pos.xy, 0, 1);
    vs_fs.uv = in_texcoord;
}
