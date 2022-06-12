#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 in_SH_1;
layout (location = 4) in vec3 in_SH_2;
layout (location = 5) in vec3 in_SH_3;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out mat3 SH_trans;

uniform mat4 m_model;
uniform mat4 m_view;
uniform mat4 m_proj;

void main()
{
    gl_Position = m_proj * m_view * m_model * vec4(aPos, 1.0f);

    FragPos = vec3(m_model * vec4(aPos, 1.0f));
    Normal = transpose(inverse(mat3(m_model))) * aNormal;
    TexCoord = aTexCoord;

    SH_trans = mat3(in_SH_1, in_SH_2, in_SH_3);
}