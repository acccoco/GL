#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 m_view;
uniform mat4 m_proj;

out vec3 TexCoord3;

void main()
{
    vec4 pos = m_proj * mat4(mat3(m_view)) * vec4(position, 1.0);
    gl_Position = vec4(pos.xy, pos.w * 0.8 , pos.w);
    TexCoord3 = position;
}