#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D tex_image;

void main() {
    FragColor = vec4(texture(tex_image, TexCoord).rgb, 1.0);
}