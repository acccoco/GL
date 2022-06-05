#version 330 core

in VS_FS {
    vec2 uv;
} vs_fs;

out vec4 out_uv;

uniform sampler2D u_tex_pos_view;
uniform sampler2D u_tex_normal_view;
uniform vec3      u_tex_size;       // 纹理的尺寸，用于得到屏幕像素
uniform mat4      u_camera_proj;


/// 避免除以零，给分母添加一个极小的数
const float EPSILON = 1e-6;


/// camera view space 的坐标转换为 uv [0, 1]^2
vec2 pos_view_to_uv(in vec3 pos_view)
{
    vec4 clip = u_camera_proj * vec4(pos_view, 1.0);
    return clip.xy / clip.w * 0.5 + 0.5;
}


/// 进行光线追踪
/// @param start_pos_view 光线的起点，确保起点是有效的
/// @param ray_dir        光线的方向，是单位向量
/// @param inter_uv       交点在 camera 坐标系下的 uv
bool ray_march(in vec3 start_pos_view, in vec3 ray_dir, out vec2 inter_uv)
{
    const int   MAX_STEPS1 = 40;            // 第一次查询的最大步数
    const int   STEP2      = 10;            // 二分查找时走的步数
    const float MAX_DIS    = 4.0;           // 最大查找距离（线性空间）
    const float RESOLUTION = 0.3;           // 第一次查找时每一步的长度，1 表示逐像素
    const float DEPTH_BIAS = 0.1;           // 视为相交的深度偏差

    /// 光线起点的各种坐标
    float start_depth = -start_pos_view.z;
    vec2 start_uv = pos_view_to_uv(start_pos_view);
    vec2  start_frag  = start_uv * u_tex_size.xy;

    /// 光线终点的各种坐标
    vec3  end_pos_view   = start_pos_view + ray_dir * MAX_DIS;
    vec2 end_uv = pos_view_to_uv(end_pos_view);
    if (end_uv.x < 0.0 || end_uv.x > 1.0 || end_uv.y < 0.0 || end_uv.y > 1.0)
        return false;
    float end_depth = -end_pos_view.z;
    if (end_depth <= 0.0)
        return false;
    vec2  end_frag  = end_uv * u_tex_size.xy;

    /// 光线线段在 screen space 的变化值
    vec2  total_delta_frag = end_frag - start_frag;

    /// 上一次和这一次 search 点位于光线线段（start-end）的百分比
    float last_search_pct = 0.0;
    float this_search_pct = 0.0;

    /// 当前 search 的点在 screen 的哪个位置
    vec2 this_frag = start_frag;


    // 第一次查找走的总步数
    float STEPS1 = max(abs(total_delta_frag.x), abs(total_delta_frag.y)) * RESOLUTION;
    STEPS1 = min(STEPS1, MAX_STEPS1);
    // 每次步进的距离（像素）
    vec2  delta_frag = total_delta_frag / max(EPSILON, STEPS1);
    int   hit1 = 0;             // 是否有交点
    for (int i = 0; i < int(STEPS1); ++i) 
    {
        this_frag += delta_frag;
        this_search_pct = abs(this_frag.x - start_frag.x) / max(EPSILON, abs(total_delta_frag.x));
        vec2 this_uv = this_frag / u_tex_size.xy;

        // 计算当前 search point 对应的深度值，需要透视矫正
        float this_depth  = start_depth * end_depth / mix(end_depth, start_depth, this_search_pct);

        // 从摄像机向当前 search point 看，可以看到的点
        float min_depth = -texture(u_tex_pos_view, this_frag / u_tex_size.xy).z;

        // 判断是否相交
        float delta_depth = this_depth - min_depth;
        if (delta_depth > 0.0 && delta_depth < DEPTH_BIAS) {
            hit1 = 1;
            break;
        } else {
            last_search_pct = this_search_pct;
        }
    }


    /// 第二次查找，使用二分查找
    int hit2 = 0;
    float last_miss = last_search_pct;      // last miss 的 percentage
    float last_hit  = this_search_pct;      // last hit  的 percentage
    this_search_pct = (last_search_pct + this_search_pct) * 0.5;
    for (int i = 0; i < STEP2 * hit1; ++i) {
        this_frag        = mix(start_frag, end_frag, this_search_pct);
        vec2 this_uv = this_frag / u_tex_size.xy;
        float this_depth = start_depth * end_depth / mix(end_depth, start_depth, this_search_pct);

        float min_depth = -texture(u_tex_pos_view, this_uv).z;

        float delta_depth = this_depth - min_depth;
        if (delta_depth > 0.0 && delta_depth < DEPTH_BIAS) {
            hit2 = 1;
            last_hit = this_search_pct;
            // 发生相交，去 last-miss 和 this-hit 中间继续找
            this_search_pct = (this_search_pct + last_miss) * 0.5;
        } else {
            // last_search_pct 就是 last-miss
            last_miss = this_search_pct;
            // 没有相交：去 this-miss 和 last-hit 中间继续找
            this_search_pct = (this_search_pct + last_hit) * 0.5;
        }
    }

    inter_uv = mix(start_frag, end_frag, last_hit) / u_tex_size.xy;
    return hit2 == 1;
}


void main()
{
    vec4 pos_view = texture(u_tex_pos_view, vs_fs.uv);
    vec4 normal_view = texture(u_tex_normal_view, vs_fs.uv);
    if (pos_view.z >= 0.0 || normal_view.w != 1.0) {
        out_uv = vec4(0.0);
        return;
    }
    vec3 light_dir_view = reflect(normalize(pos_view.xyz), normalize(normal_view.xyz));

    vec2 inter_uv;
    if (ray_march(pos_view.xyz, light_dir_view, inter_uv)) {
        out_uv = vec4(inter_uv, 1.0, 1.0);
    } else {
        out_uv = vec4(0.0);
    }
}