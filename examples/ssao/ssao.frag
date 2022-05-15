#version 330 core

in VS_FS {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} vs_fs;

out vec4 FragColor;

uniform sampler2D tex_geometry;
uniform bool      has_diffuse;
uniform sampler2D tex_diffuse;
uniform vec3      kd;
uniform mat4      camera_vp;
uniform vec3      light_pos;
uniform vec3      rand_seed3;

const float PI   = 3.141592653589793;
const float _2PI = 6.283185307179586;

/// 默认的环境光照
const vec3  ambient_Li  = vec3(0.3);
const float ssao_radius = 0.3;
const int   NUM_RINGS   = 5;
const int   NUM_SAMPLES = 17;

/// 计算 TBN 矩阵
mat3 gen_TBN(vec3 N);

/// 根据二维随机数生成一个半球方向
vec3 rand_to_hemisphere(vec2 rand2);

/// 直接光照的着色
vec3 direct_shading(vec3 albedo);

/// 进行 SSAO 的间接光照着色
vec3 ssao(mat3 TBN, vec3 albedo);

vec2 poisson_disk[NUM_SAMPLES];
void gen_poisson_disk(vec2 rand_seed2);

void main() {
    vec3 albedo = has_diffuse ? texture(tex_diffuse, vs_fs.TexCoord).rgb : kd;
    mat3 TBN = gen_TBN(normalize(vs_fs.Normal));
    gen_poisson_disk(rand_seed3.xy + gl_FragCoord.xy);

    vec3 direct_color = direct_shading(albedo);
    vec3 ambient_color = ssao(TBN, albedo);

    FragColor = vec4(ambient_color + direct_color, 1.0);
}

vec3 direct_shading(vec3 albedo)
{
    vec3 dir_L = normalize(light_pos - vs_fs.FragPos);
    float cos_theta = max(0.0, dot(dir_L, normalize(vs_fs.Normal)));
    return albedo / 2.0 * cos_theta;
}

/// 根据二维向量生成随机数
float rand2(vec2 xy)
{
    float dt = dot(xy, vec2(12.9898, 78.233));
    float sn = mod(dt, PI);
    return fract(sin(sn) * 43758.5453);
}

void gen_poisson_disk(vec2 rand_seed2)
{
    const float ANGLE_STEP = _2PI * float(NUM_RINGS) / float(NUM_SAMPLES);

    float angle = rand2(rand_seed2) * _2PI;// init angle
    float radius= 1.0 / float(NUM_SAMPLES);// init radius and radius step
    float radius_step = radius;

    for (int i = 0; i < NUM_SAMPLES; ++i) {
        poisson_disk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
        radius += radius_step;
        angle += ANGLE_STEP;
    }
}

vec3 ssao(mat3 TBN, vec3 albedo)
{
    float sum = 0.0;/// 分子
    float base = 0.0;/// 加权平均的分母

    /// 将采样点记为 S
    for (int i = 0; i < 16; ++i)
    {
        /// 获得随机的采样点
        vec2 rand2 = poisson_disk[i] * 0.5 + 0.5;/// 随机数范围是 [0, 1]
        vec3 pos_to_s = TBN * rand_to_hemisphere(rand2);
        vec3 s_pos = pos_to_s * ssao_radius + vs_fs.FragPos;

        /// 计算采样点的深度
        vec4 s_clip = camera_vp * vec4(s_pos, 1.0);
        vec2 s_tex_coord = (s_clip.xy / s_clip.w) * 0.5 + 0.5;
        float s_depth = s_clip.w;
        float s_depth_min = texture(tex_geometry, s_tex_coord).w;

        /// 累计结果
        float cos_theta = dot(normalize(vs_fs.Normal), pos_to_s);
        if (s_depth + 0.001 < s_depth_min || s_depth - s_depth_min > 0.8)
        sum += cos_theta;
        base += cos_theta;
    }

    float kA = sum / base;
    return kA * ambient_Li * albedo;
}


mat3 gen_TBN(vec3 N)
{
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    return mat3(tangent, bitangent, N);
}

vec3 rand_to_hemisphere(vec2 rand2)
{
    float phi = 2.0 * PI * rand2.x;
    float theta = acos(1.0 - rand2.y);

    float sin_theta = sin(theta), cos_theta = cos(theta);
    float sin_phi = sin(phi), cos_phi = cos(phi);

    return vec3(sin_theta * cos_phi, sin_theta * sin_phi, cos_theta);
}