#version 330 core

const float PI = 3.141592653589793;

in VS_FS
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} vs_fs;

uniform vec3      light_indensity;
uniform float     fov_deg;              // 摄像机的视角
uniform float     viewport_size;        // 近平面的尺寸
uniform vec3      kd;
uniform bool      has_diffuse;
uniform sampler2D tex_diffuse;
uniform float     zoom_in;              // flux 的放大倍数

layout (location = 0) out vec4 pos_depth;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec3 flux;


/// 距摄像机 1.0 远的平面上某个点与其原点围成的矩形对应的立体角是多少
float total_solid_angle(vec2 uv);

/// 计算像素对应的立体角
float pixel_solid_angle();


void main()
{
    pos_depth = vec4(vs_fs.FragPos, 1.0 / gl_FragCoord.w);
    normal = vs_fs.Normal;

    vec3 albedo = has_diffuse ? texture(tex_diffuse, vs_fs.TexCoord).rgb : kd;
    flux = albedo / PI * light_indensity * pixel_solid_angle();
}

float total_solid_angle(vec2 uv)
{
    /// lim_{x->0}: arctan(x) ~ x
    // return atan(uv.x * uv.y, sqrt(uv.x * uv.x + uv.y * uv.y + 1.0));
    // zoom in
    float zoom_in_sqrt = sqrt(zoom_in);
    return (uv.x * zoom_in_sqrt) * (uv.y * zoom_in_sqrt) / sqrt(uv.x * uv.x + uv.y * uv.y + 1.0);
}

float pixel_solid_angle()
{
    float size_inverse = 1.0 / viewport_size;
    float tan_half_fov = tan(fov_deg / 180.0 * PI / 2.0);
    /// viewport 坐标系转化为 NDC
    vec2 NDC = gl_FragCoord.xy * size_inverse * 2.0 - 1.0;

    /// NDC 转换为距摄像机 1.0 的平面上，像素的中心点
    vec2 uv = NDC * tan_half_fov;

    /// 像素 4 个 cornel 在 1.0 远的平面上对应的坐标
    float half_resolution = size_inverse * tan_half_fov;
    vec2 A = uv + vec2(-half_resolution);
    vec2 B = uv + vec2(half_resolution, -half_resolution);
    vec2 C = uv + vec2(-half_resolution, half_resolution);
    vec2 D = uv + vec2(half_resolution);

    return total_solid_angle(D) - total_solid_angle(B) - total_solid_angle(C) + total_solid_angle(A);
}