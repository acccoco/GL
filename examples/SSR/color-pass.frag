#version 330 core

const float PI = 3.141592653589793238;
const float g_roughness = 0.2;
const float DELTA = 1e-6;
const uint  SAMPLE_CNT = 8u;        // 采样的数量

in VS_FS {
    vec3 world_pos;
    vec3 world_normal;
    vec2 tex_coord;
    vec4 camera_clip;
} vs_fs;

out vec4 out_fragcolor;

uniform mat4 u_camera_vp_;
uniform vec3 u_camera_pos;

uniform sampler2D u_tex_depth_visibility;   // 摄像机坐标系下的 depth 和 visibility
uniform sampler2D u_tex_world_pos;
uniform sampler2D u_tex_world_normal;
uniform sampler2D u_tex_direct_color;

uniform bool      u_has_diffuse;
uniform vec3      u_kd;
uniform sampler2D u_tex_diffuse;


/**
 * 将世界坐标系转换为摄像机内的 uv
 * @param in  world_pos    世界坐标系中的位置
 * @param out camera_uv    在 camera 坐标系下的 uv
 * @return 返回在 camera 坐标系下的深度
 */
float camera_uv_depth(in vec3 world_pos, out vec2 camera_uv)
{
    vec4 camera_clip = u_camera_vp_ * vec4(world_pos, 1.0);
    camera_uv = camera_clip.xy / camera_clip.w * 0.5 + 0.5;
    camera_uv = clamp(camera_uv, vec2(0.0), vec2(1.0));
    return camera_clip.w;
}


/**
 * 进行光线追踪
 * @param in  ray_origin 光线的起点
 * @param in  ray_dir    光线的方向，是单位向量
 * @param out inter_uv   交点在 camera 坐标系下的 uv
 */
bool ray_march(in vec3 ray_origin, in vec3 ray_dir, out vec2 inter_uv)
{
    const int   MAX_STEPS = 20;             // 最多走的步数
    const float STEP_LEN = 0.1;             // 每步的长度
    
    bool  is_inter = false;                 // 是否和场景有交点
    float ray_dis  = 0.0;                   // 光线走过的距离
    for (int _ = 1; _ <= MAX_STEPS; ++_)
    {
        ray_dis += STEP_LEN;
        vec3 ray_end_pos = ray_origin + ray_dir * ray_dis;
        vec2 uv_in_camera;
        float depth_in_camera = camera_uv_depth(ray_end_pos, uv_in_camera);
        float depth_in_camera_min = texture(u_tex_depth_visibility, uv_in_camera).r;
        if (depth_in_camera > depth_in_camera_min) {
            is_inter = true;
            break;
        }
    }

    camera_uv_depth(ray_origin + ray_dir * ray_dis, inter_uv);
    return is_inter;
}


/// radical inverse van der corput
float radical_inverse_VDC(in uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;// / 0x100000000
}


/// 低差异序列
vec2 Hammersley(in uint i, in uint N)
{
    return vec2(float(i)/float(N), radical_inverse_VDC(i));
}


/**
 * GGX 采样
 * @param Xi 二维的随机数 [0, 1]^2
 */
vec3 importance_sample_GGX(in vec2 Xi, in float roughness, in vec3 N, out float pdf)
{
    float a = roughness * roughness;    // better visual effect
    float a2 = a * a;

    // uniform distribution to GGX distribution
    float phi = 2 * PI * Xi.x;
    float cos_theta = sqrt((1.0 - Xi.y) / (1 + (a2 - 1) * Xi.y));
    float cos_theta2 = cos_theta * cos_theta;
    float sin_theta = sqrt(1 - cos_theta2);

    // half-way vector in tagent space
    vec3 H;
    H.x = sin_theta * cos(phi);
    H.y = sin_theta * sin(phi);
    H.z = cos_theta;

    // tangent space base vector
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    // pdf
    float deno = PI * pow((1 + cos_theta2 * (a2 - 1.0)), 2.0) + DELTA;
    pdf = a2 * sin_theta * cos_theta / deno + DELTA;

    // tangent space to world space
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}


/**
 * 利用次级光源进行着色
 */
vec3 indirect_shading(in vec3 albedo)
{
    vec3 v = normalize(u_camera_pos - vs_fs.world_pos);
    vec3 n = normalize(vs_fs.world_normal);

    vec3 color = vec3(0.0);
    for (uint i = 0u; i < SAMPLE_CNT; ++i) {
        float pdf;
        vec3 h = importance_sample_GGX(Hammersley(i, SAMPLE_CNT), g_roughness, n, pdf);
        vec3 light_dir = normalize(reflect(h, -v));

        vec2 inter_uv;
        if (!ray_march(vs_fs.world_pos, light_dir, inter_uv)) 
            continue;

        vec3 inter_pos = texture(u_tex_world_pos, inter_uv).xyz;
        vec3 l = normalize(inter_pos - vs_fs.world_pos);
        vec3 inter_normal = normalize(texture(u_tex_world_normal, inter_uv).xyz);

        /// 次级光源的光无法照射到当前着色点
        if (dot(inter_normal, -l) <= 0.0) 
            continue;
        
        vec3 Li = texture(u_tex_direct_color, inter_uv).rgb;
        color += Li * albedo / PI * max(0.0, dot(n, l)) * 10.0;  // TODO pdf
    }
    
    return color / float(SAMPLE_CNT);
}


void main()
{
    vec3 albedo = u_has_diffuse ? texture(u_tex_diffuse, vs_fs.tex_coord).rgb : u_kd;

    vec2 uv_in_camera = vs_fs.camera_clip.xy / vs_fs.camera_clip.w * 0.5 + 0.5;
    vec3 direct_color = texture(u_tex_direct_color, uv_in_camera).rgb;

    vec3 indirect_color = indirect_shading(albedo);

    out_fragcolor = vec4(direct_color + indirect_color, 1.0);
}