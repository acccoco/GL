#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 frag_clip_light_coord;

uniform mat4 m_model;
uniform mat4 m_view;
uniform mat4 m_proj;
uniform mat4 light_vp;

void main() {
    gl_Position = m_proj * m_view * m_model * vec4(aPos, 1.0f);

    FragPos = vec3(m_model * vec4(aPos, 1.0f));
    Normal = transpose(inverse(mat3(m_model))) * aNormal;
    TexCoord = aTexCoord;

    frag_clip_light_coord = light_vp * m_model * vec4(aPos, 1.0f);
}