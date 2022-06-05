#version 330 core

layout (location = 0) in vec3 in_pos;

uniform mat4 u_light_mvp;


void main()
{
    gl_Position = u_light_mvp * vec4(in_pos, 1.0);
}