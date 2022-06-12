#version 330 core

in VS_FS
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} vs_fs;

out vec4 FragColor;

uniform bool has_diffuse;
uniform sampler2D tex_diffuse;
uniform vec3 kd;


void main()
{
    if (has_diffuse)
    {
        FragColor = vec4(texture(tex_diffuse, vs_fs.TexCoord).rgb, 1.0);
    }
    else
    {
        FragColor = vec4(pow(kd, vec3(1.0 / 2.2)), 1.0);
    }
}