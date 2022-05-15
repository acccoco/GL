#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 Geomrtry;


void main() {
    Geomrtry = vec4(Normal, 1.0 / gl_FragCoord.w);
}