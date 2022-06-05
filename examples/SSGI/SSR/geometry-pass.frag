#version 330 core

in VS_FS {
    vec3 pos_view;
    vec3 normal_view;
    vec2 uv;
} vs_fs;

layout (location = 0) out vec4 out_pos_view;
layout (location = 1) out vec4 out_normal_view;
layout (location = 2) out vec4 out_diffuse;

uniform bool      u_has_diffuse;
uniform vec3      u_kd;
uniform sampler2D u_tex_diffuse;


void main()
{
    vec3 albedo = u_has_diffuse ? texture(u_tex_diffuse, vs_fs.uv).rgb : u_kd;
    
    out_diffuse = vec4(albedo, 1.0);
    out_pos_view = vec4(vs_fs.pos_view, 1.0);
    out_normal_view = vec4(normalize(vs_fs.normal_view), 1.0);
}
