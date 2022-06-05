#version 330 core

in vec4 pos_in_clip;

out vec4 FragColor;

// make sure texture type GL_FLOAT and internal_format GL_RGB
void main()
{
    // FragColor = vec4(vec3(pos_in_clip.w), 1.0);
    FragColor = vec4(vec3(1.0 / gl_FragCoord.w), 1.0);
}