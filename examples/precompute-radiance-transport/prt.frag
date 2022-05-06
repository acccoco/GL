#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 SH_trans;

uniform mat3 SH_light_R;
uniform mat3 SH_light_G;
uniform mat3 SH_light_B;

out vec4 FragColor;

void main()
{
    float r = dot(SH_trans[0], SH_light_R[0]) + dot(SH_trans[1], SH_light_R[1]) + dot(SH_trans[2], SH_light_R[2]);
    float g = dot(SH_trans[0], SH_light_G[0]) + dot(SH_trans[1], SH_light_G[1]) + dot(SH_trans[2], SH_light_G[2]);
    float b = dot(SH_trans[0], SH_light_B[0]) + dot(SH_trans[1], SH_light_B[1]) + dot(SH_trans[2], SH_light_B[2]);
    FragColor = vec4(pow(vec3(r, g, b) * 1.0, vec3(1.0/2.2)), 1.0);
    // FragColor = FragColor * 0.0001 + vec4(1.0);
}