#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoord3;

uniform mat4 m_view;
uniform mat4 m_proj;

void main()
{
    vec4 pos = m_proj * mat4(mat3(m_view)) * vec4(aPos, 1.0);
    gl_Position = vec4(pos.xy, pos.w * 0.999, pos.w);
    TexCoord3 = aPos;
}