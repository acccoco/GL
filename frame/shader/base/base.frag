#version 330 core

/// 显示灰模的着色器
/// 光源就是摄像机

in VS_FS {
    vec3 normal;
    vec3 pos_world;
} vs_fs;

out vec4 out_color;

uniform vec3 u_camera_pos;

const vec3 light_color = vec3(0.7, 0.7, 0.7);


void main()
{
    vec3 L = normalize(u_camera_pos - vs_fs.pos_world);
    vec3 N = normalize(vs_fs.normal);
    vec3 V = L, H = L;

    vec3 color_ambient = vec3(0.35);
    vec3 color_diffuse = vec3(0.2) * abs(dot(L, N));
    vec3 color_specular = vec3(0.07) * pow(abs(dot(N, H)), 16);

    out_color = vec4(color_ambient + color_diffuse + color_specular, 1.0);
}