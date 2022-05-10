#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VS_FS
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} vs_fs;

uniform mat4 m_model;
uniform mat4 m_view;
uniform mat4 m_proj;

void main() {
    gl_Position = m_proj * m_view * m_model * vec4(aPos, 1.0f);

    vs_fs.FragPos = vec3(m_model * vec4(aPos, 1.0f));
    vs_fs.Normal = transpose(inverse(mat3(m_model))) * aNormal;
    vs_fs.TexCoord = aTexCoord;
}