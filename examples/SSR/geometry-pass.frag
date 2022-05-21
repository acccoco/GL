#version 330 core

in VS_FS {
    vec3 world_pos;
    vec3 world_normal;
    vec2 uv;
} vs_fs;

layout (location = 0) out vec4 out_world_pos;
layout (location = 1) out vec4 out_world_normal;
layout (location = 2) out vec4 out_depth_visibility;
layout (location = 3) out vec4 out_direct_color;

uniform bool      u_has_diffuse;
uniform vec3      u_kd;
uniform sampler2D u_tex_diffuse;

uniform sampler2D u_shadow_map;
uniform vec3      u_light_dir;
uniform vec3      u_light_color;
uniform mat4      u_light_vp;

/// 进行 diffuse 着色
vec3 shading_diffuse(in vec3 n);

/// 利用 shadow map 计算 visibility
float visibility_get();


void main()
{
    vec3 n = normalize(vs_fs.world_normal);
    
    float visibility = visibility_get();

    out_world_pos = vec4(vs_fs.world_pos, 1.0);
    out_world_normal = vec4(n, 1.0);
    out_depth_visibility = vec4(1.0 / gl_FragCoord.w, visibility, 0.0, 1.0);

    out_direct_color = vec4(shading_diffuse(n) * visibility, 1.0);
}


/// ==================================================================

vec3 shading_diffuse(in vec3 n)
{
    vec3 l = normalize(-u_light_dir);
    float n_dot_l = max(0.0, dot(l, n));
    vec3 albedo = u_has_diffuse ? texture(u_tex_diffuse, vs_fs.uv).rgb : u_kd;
    return n_dot_l * u_light_color * albedo;
}

float visibility_get()
{
    float bias = 1e-3;  /// TODO bias 设定为定值 
    float depth_in_light;
    float depth_in_shadowmap;
    {
        vec4 clip_in_light = u_light_vp * vec4(vs_fs.world_pos, 1.0);
        vec3 NDC_in_light = clip_in_light.xyz / clip_in_light.w;

        depth_in_shadowmap = texture(u_shadow_map, clamp(NDC_in_light.xy * 0.5 + 0.5, vec2(0.0), vec2(1.0))).r;
        depth_in_light = NDC_in_light.z * 0.5 + 0.5;       // 正交投影，
    }
    return depth_in_light < depth_in_shadowmap + bias ? 1.0 : 0.0;
}