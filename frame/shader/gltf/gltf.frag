#version 330 core

/// gltf 的材质系统

in VS_FS {
    vec3 pos_view;
    vec3 normal_view;
    vec2 texcoord_0;
    mat3 TBN_view;
} vs_fs;

out vec4 out_color;

/// basecolor 
uniform vec4 u_basecolor;
uniform bool u_has_basecolor;
uniform sampler2D u_tex_basecolor;

/// occlusion
uniform float u_occlusion_strength;
uniform bool u_has_occlusion;
uniform sampler2D u_tex_occlusion;

/// emissive
uniform vec3 u_emissive;
uniform bool u_has_emissive;
uniform sampler2D u_tex_emissive;

/// point light 
uniform vec3 u_light_pos;
uniform vec3 u_light_color;

/// N map 
uniform bool u_has_normal;
uniform sampler2D u_tex_normal;
uniform float u_normal_scale;


void main()
{
    /// basecolor 
    vec4 basecolor = u_basecolor;
    if (u_has_basecolor) {
        basecolor = texture(u_tex_basecolor, vs_fs.texcoord_0);
    }

    /// occlused basecolor 
    if (u_has_occlusion) {
        float occlusion = texture(u_tex_occlusion, vs_fs.texcoord_0).r;
        basecolor.rgb = mix(basecolor.rgb, basecolor.rgb * occlusion, u_occlusion_strength);
    }

    /// emissive 
    vec3 emissive = u_emissive;
    if (u_has_emissive) {
        emissive = texture(u_tex_emissive, vs_fs.texcoord_0).rgb;
    }

    /// normal 
    vec3 N;
    if (u_has_normal) {
        vec3 normal_tangent = texture(u_tex_normal, vs_fs.texcoord_0).xyz * 2.0 - 1.0;
        normal_tangent = normalize(normal_tangent * vec3(u_normal_scale, u_normal_scale, 1.0));
        N = vs_fs.TBN_view * normal_tangent;
    }
    else 
        N = normalize(vs_fs.normal_view);

    /// blinn-phong
    vec3 phong_color;
    {
        vec3 L = normalize(u_light_pos - vs_fs.pos_view);
        vec3 V = normalize(-vs_fs.pos_view);
        vec3 H = normalize(L + V);

        vec3 diffuse = max(0.0, dot(N, L)) * u_light_color * basecolor.xyz;
        vec3 specular = pow(max(0.0, dot(N, H)), 32) * u_light_color * basecolor.xyz;

        phong_color = diffuse + specular;
    }

    /// 丢弃透明的
    if (basecolor.a < 0.1) discard;
    out_color = vec4(phong_color + emissive, basecolor.a);
}