#version 330 core

/// 用于绘制线条的着色器

in VS_FS {
    vec3 color;
} vs_fs;

out vec4 out_color;


void main()
{
    out_color = vec4(vs_fs.color, 1.0);
}