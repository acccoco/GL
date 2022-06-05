#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D tex_image;
uniform int       channel;

void main() {
    vec4 value = texture(tex_image, TexCoord);
    switch (channel)
    {
        case 0: FragColor = vec4(value.rgb, 1.0); break;
        case 1: FragColor = vec4(vec3(value.r), 1.0); break;
        case 2: FragColor = vec4(vec3(value.g), 1.0); break;
        case 3: FragColor = vec4(vec3(value.b), 1.0); break;
        case 4: FragColor = vec4(vec3(value.a), 1.0); break;
        default: FragColor = vec4(value.rgb, 1.0);
    }
}