#version 330 core

/// gltf 的材质系统


layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord_0;
layout (location = 3) in vec4 in_tangent;

out VS_FS {
    vec3 pos_view;
    vec3 normal_view;
    vec2 texcoord_0;
    mat3 TBN_view;
} vs_fs;

uniform mat4 u_model_view;
uniform mat4 u_proj;


void main() {
    vec4 temp = u_model_view * vec4(in_pos, 1.0);
    gl_Position = u_proj * temp;

    vs_fs.pos_view = temp.xyz / temp.w;
    mat3 matrix_mv_it = transpose(inverse(mat3(u_model_view)));
    vs_fs.normal_view = normalize(matrix_mv_it * in_normal);
    vs_fs.texcoord_0 = in_texcoord_0;

    vec3 tangent = normalize(matrix_mv_it * in_tangent.xyz);
    vec3 bit_tangent = cross(vs_fs.normal_view, tangent) * in_tangent.w;  // w 分量是 -1 或 1
    vs_fs.TBN_view = mat3(tangent, bit_tangent, vs_fs.normal_view);
}
