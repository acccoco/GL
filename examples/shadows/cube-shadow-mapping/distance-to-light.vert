#version 330 core

layout (location = 0) in vec3 pos;

out vec3 pos_in_view_coord;

uniform mat4 m_model;
uniform mat4 m_view;
uniform mat4 m_proj;

void main()
{
    gl_Position = m_proj * m_view * m_model * vec4(pos, 1.0);
    pos_in_view_coord = vec3(m_view * m_model * vec4(pos, 1.0));
}