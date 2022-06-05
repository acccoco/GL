#version 330 core 

in VS_FS {
    vec2 uv;
} vs_fs;

out vec4 out_fragcolor;

uniform sampler2D u_tex_pos_view;
uniform sampler2D u_tex_normal_view;
uniform sampler2D u_tex_diffuse;
uniform sampler2D u_shadow_map;
uniform sampler2D u_tex_ssr_uv;

uniform mat4 u_view_to_light;   // 从 camera 的 view space 变换到 light 的 clip space
uniform vec3 u_light_dir_view;  // camera 的 view space 中 light 的方向
uniform vec3 u_light_color;

/// ambient，diffuse，speuclar 的系数
const float ka = 0.3;
const float kd = 0.5;
const float ks = 0.2;

/// 直接光照着色，blinn-phong
vec3 shading_direct(in vec3 n, in vec3 l, in vec3 v, in vec3 albedo)
{
    /// diffuse
    vec3 diffuse = kd * u_light_color * albedo * max(0.0, dot(n, l));
    
    /// specular
    vec3 h = normalize(l + v);
    vec3 specular = ks * u_light_color * albedo * pow(max(0.0, dot(n, h)), 32);

    return diffuse + specular;
}


/// 默认在 view space 中
void main()
{
    vec4 pos = texture(u_tex_pos_view, vs_fs.uv);
    /// 如果 position 纹理的 w 通道是 0，说明这个点是背景
    if (pos.w != 1.0) {
        out_fragcolor = vec4(0.0);
        return;
    }

    vec3 v = normalize(-pos.xyz);
    vec3 n = normalize(texture(u_tex_normal_view, vs_fs.uv).xyz);
    vec3 albedo = texture(u_tex_diffuse, vs_fs.uv).xyz;

    /// 直接光源着色
    vec3 l_direct = normalize(-u_light_dir_view);
    vec3 color_direct = shading_direct(n, l_direct, v, albedo);

    /// 间接光源着色
    vec4 ssr_uv = texture(u_tex_ssr_uv, vs_fs.uv);
    vec3 color_indirect = vec3(0.0);
    if (ssr_uv.w != 1.0) {  // w 通道是 0 说明找不到反射点的 uv 
        vec3 indirect_normal = normalize(texture(u_tex_normal_view, ssr_uv.xy).xyz);
        vec3 indirect_pos = texture(u_tex_pos_view, ssr_uv.xy).xyz;
        vec3 l_indirect = normalize(indirect_pos - pos.xyz);
        vec3 indirect_albedo = texture(u_tex_diffuse, ssr_uv.xy).xyz;
        
        // 确保间接光照是从上半球发出的
        if (dot(-l_indirect, indirect_normal) > 0.0) {
            color_indirect = max(0.0, dot(indirect_normal, l_direct)) * albedo * max(0.0, dot(n, l_indirect)) * indirect_albedo;
        }
    } 

    out_fragcolor = vec4(color_direct + color_indirect, 1.0);
}