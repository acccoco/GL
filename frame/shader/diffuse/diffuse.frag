#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform bool has_diffuse;
uniform sampler2D tex_diffuse;
uniform vec3 kd;

void main() {
    FragColor = has_diffuse ? texture(tex_diffuse, TexCoord) : vec4(pow(kd, vec3(1.0/2.2)), 1.0);
}