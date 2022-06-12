#version 330 core

uniform samplerCube tex_cube;
uniform float roughness;
uniform int total_mip_level;

in vec3 TexCoord3;
out vec4 FragColor;

void main()
{
    float mip_level = roughness * float(total_mip_level - 1);
    FragColor = vec4(textureLod(tex_cube, TexCoord3, mip_level).rgb, 1.0);
}