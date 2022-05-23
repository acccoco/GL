#version 330 core

const float PI = 3.141592653589793238;
const float g_roughness = 0.6;
const float DELTA = 1e-6;
const uint  SAMPLE_CNT = 1u;        // 采样的数量

in VS_FS {
    vec3 world_pos;
    vec3 world_normal;
    vec2 tex_coord;
    vec4 camera_clip;
} vs_fs;

out vec4 out_fragcolor;

uniform mat4 u_camera_vp_;
uniform vec3 u_camera_pos;

uniform sampler2D u_tex_world_pos;
uniform sampler2D u_tex_world_normal;
uniform sampler2D u_tex_direct_color;
uniform vec3      u_tex_size;

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
    return camera_clip.w;
}


/**
 * 进行光线追踪
 * @param in  ray_origin 光线的起点
 * @param in  ray_dir    光线的方向，是单位向量
 * @param out inter_uv   交点在 camera 坐标系下的 uv
 */
bool ray_march(in vec3 ray_origin_pos, in vec3 ray_dir, out vec2 inter_uv)
{
    const int   STEP2      = 10;            // 二分查找时走的步数
    const float MAX_DIS    = 4.0;           // 最大查找距离（线性空间）
    const float RESOLUTION = 0.1;           // 第一次查找时每一步的长度，1 表示逐像素
    const float DEPTH_BIAS = 0.2;           // 视为相交的深度偏差

    vec2  ray_origin_uv;                                                    // 光线起点在 view space 中的 uv 
    float ray_origin_depth = camera_uv_depth(ray_origin_pos, ray_origin_uv);
    vec2  ray_origin_frag  = ray_origin_uv * u_tex_size.xy;
    vec3  ray_end_pos   = ray_origin_pos + ray_dir * MAX_DIS;               // 光线的终点
    vec2  ray_end_uv;                                                       // 光线终点在 view space 中的 uv 
    float ray_end_depth = camera_uv_depth(ray_end_pos, ray_end_uv);
    vec2  ray_end_frag  = ray_end_uv * u_tex_size.xy;
    vec2  delta_frag = ray_end_frag - ray_origin_frag;                      // 光线线段在 screen space 的变化值

    float last_search = 0.0;        // 上一次 search 时位于光线线段的百分比（screen space）
    float this_search = 0.0;        // 当前 search 时位于光线线段的白分比
    vec2  cur_frag    = ray_origin_frag;


    /// 第一次查找，每次的步长是相同的
    float steps1    = max(abs(delta_frag.x), abs(delta_frag.y)) * RESOLUTION;   // 第一次查找走的总步数
    vec2  increment = delta_frag / max(0.001, steps1);                          // 每次步进的距离（像素）
    int   hit1 = 0;             // 是否有交点
    for (int i = 0; i < int(steps1); ++i) {
        cur_frag    += increment;
        this_search = (cur_frag.x - ray_origin_frag.x) / delta_frag.x;
        vec2 cur_uv = cur_frag / u_tex_size.xy;

        // 计算当前 search point 对应的深度值，需要透视矫正
        float cur_depth  = ray_origin_depth * ray_end_depth / mix(ray_end_depth, ray_origin_depth, this_search);

        // 从摄像机向当前 search point 看，可以看到的点
        vec3  min_pos   = texture(u_tex_world_pos, cur_uv).xyz;
        float min_depth = (u_camera_vp_ * vec4(min_pos, 1.0)).w;

        // 判断是否相交
        float delta_depth = cur_depth - min_depth;
        if (delta_depth > 0.0 && delta_depth < DEPTH_BIAS) {
            hit1 = 1;
            break;
        } else {
            last_search = this_search;
        }
    }


    /// 第二次查找，使用二分查找
    int hit2 = 0;
    float last_miss = last_search;      // last miss 的 percentage
    float last_hit  = this_search;      // last hit  的 percentage
    this_search = (last_search + this_search) * 0.5;
    for (int i = 0; i < STEP2 * hit1; ++i) {
        cur_frag        = mix(ray_origin_frag, ray_end_frag, this_search);
        vec2 cur_uv     = cur_frag / u_tex_size.xy;
        float cur_depth = ray_origin_depth * ray_end_depth / mix(ray_end_depth, ray_origin_depth, this_search);

        vec3 min_pos    = texture(u_tex_world_pos, cur_uv).xyz;
        float min_depth = (u_camera_vp_ * vec4(min_pos, 1.0)).w;

        float delta_depth = cur_depth - min_depth;
        if (delta_depth > 0.0 && delta_depth < DEPTH_BIAS) {
            hit2 = 1;
            last_hit = this_search;
            // 发生相交，去 last-miss 和 this-hit 中间继续找
            this_search = (this_search + last_miss) * 0.5;
        } else {
            // last_search 就是 last-miss
            last_miss = this_search;
            // 没有相交：去 this-miss 和 last-hit 中间继续找
            this_search = (this_search + last_hit) * 0.5;
        }
    }

    inter_uv = mix(ray_origin_frag, ray_end_frag, this_search) / u_tex_size.xy;
    return hit2 == 1;
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
        vec3 h = n;
        vec3 light_dir = normalize(reflect(-v, -h));

        vec2 inter_uv;
        if (!ray_march(vs_fs.world_pos, light_dir, inter_uv)) 
            continue;
        if (inter_uv.x < 0.0 || inter_uv.x > 1.0 || inter_uv.y < 0.0 || inter_uv.y > 1.0)
            continue;

        vec3 inter_pos = texture(u_tex_world_pos, inter_uv).xyz;
        vec3 l = normalize(inter_pos - vs_fs.world_pos);
        vec3 inter_normal = normalize(texture(u_tex_world_normal, inter_uv).xyz);

        /// 次级光源的光无法照射到当前着色点
        if (dot(inter_normal, -l) <= 0.0) 
            continue;
        
        vec3 Li = texture(u_tex_direct_color, inter_uv).rgb;
        color += Li * albedo / PI * max(0.0, dot(n, l)) * 10.0;  // FIXME pdf
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