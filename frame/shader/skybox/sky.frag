#version 330 core

uniform samplerCube tex_cube;

in vec3 TexCoord3;
out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(tex_cube, TexCoord3).rgb, 1.0);
}

