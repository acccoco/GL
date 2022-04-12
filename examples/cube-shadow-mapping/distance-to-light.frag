#version 330 core

in vec3 pos_in_view_coord;
out vec4 FragColor;

// make sure texture type GL_FLOAT and internal_format GL_RGB
// depth is the length from light to frag
void main()
{
    float dis = length(pos_in_view_coord);
    FragColor = vec4(vec3(1, 1, 1) * dis, 1.0);
}