#version 330 core

const float PI   = 3.141592653589793;
const float _2PI = 6.283185307179586;
const int NUM_RINGS = 19;
const int NUM_SAMPLES = 201;
const float light_indensity = 1.3f;

in VS_FS
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} vs_fs;

uniform mat4      light_VP;
uniform sampler2D RSM_pos;
uniform sampler2D RSM_normal;
uniform sampler2D RSM_flux;
uniform float     flux_zoom_in; // flux 放大的倍数
uniform vec3      random_seed;
uniform vec3      light_pos;
uniform bool      has_diffuse;
uniform sampler2D tex_diffuse;
uniform vec3      kd;

out vec4 FragColor;


/// 根据种子生成随机数
float rand2(vec2 xy);

/// 生成 poisson 圆盘，随机数的点范围是 [0, 1] * [0, 1]
vec2 poisson_disk[NUM_SAMPLES];
void gen_poisson_disk(vec2 rand_seed);

vec3 direct_illuminate(vec3 view, vec3 normal);

vec3 indirect_illuminate(vec3 normal, vec2 uv);

void main()
{
    vec3 view = normalize(light_pos - vs_fs.FragPos);
    vec3 normal = normalize(vs_fs.Normal);

    vec3 direct_illuminace = direct_illuminate(view, normal);

    /// 将片段投影到 RSM 中，得到对应的纹理坐标
    vec4 clip_coord = light_VP * vec4(vs_fs.FragPos, 1.0);
    vec2 uv = clip_coord.xy / clip_coord.w * 0.5 + 0.5;

    gen_poisson_disk(random_seed.xy);
    vec3 indirect_illuminace = indirect_illuminate(normal, uv);

    FragColor = vec4(direct_illuminace + indirect_illuminace, 1.0);
}



float rand2(vec2 xy)
{
    float dt = dot(xy, vec2(12.9898, 78.233));
    float sn = mod(dt, PI);
    return fract(sin(sn) * 43758.5453);
}

void gen_poisson_disk(vec2 rand_seed)
{
    const float ANGLE_STEP = _2PI * float(NUM_RINGS) / float(NUM_SAMPLES);

    float angle = rand2(rand_seed) * _2PI;// init angle
    const float RADIUS_STEP = 1.0 / float(NUM_SAMPLES);
    float radius= RADIUS_STEP;

    for (int i = 0; i < NUM_SAMPLES; ++i) {
        /// TODO: 修改点的聚集程度
        poisson_disk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.85);
        radius += RADIUS_STEP;
        angle += ANGLE_STEP;
    }
}


vec3 direct_illuminate(vec3 view, vec3 normal)
{
    vec3 albedo = has_diffuse ? texture(tex_diffuse, vs_fs.TexCoord).rgb : kd;
    // return max(0.0, dot(view, normal)) * albedo / PI * light_indensity;
    return albedo / PI * light_indensity;
}


vec3 indirect_illuminate(vec3 normal, vec2 uv)
{
    /// 视为 diffuse 的着色
    vec3 albedo = has_diffuse ? texture(tex_diffuse, vs_fs.TexCoord).rgb : kd;
    vec3 fr = albedo / PI;

    /// 将 RSM 上的每个像素都视为一个次级光源
    /// 次级光源记作 Q，当前着色的片段记作 P
    vec3 shading_sum = vec3(0.0);
    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        vec2 uv_q = uv + poisson_disk[i];                           // Q 在 RSM 上的纹理坐标
        vec3 pos_q = texture(RSM_pos, uv_q).xyz;                    // Q 的实际位置
        vec3 normal_q = normalize(texture(RSM_normal, uv_q).xyz);   // Q 的法线
        vec3 q_to_p = normalize(vs_fs.FragPos - pos_q);             // 向量：Q -> P
        float dis = distance(vs_fs.FragPos, pos_q);

        float cos_p = max(0.0, dot(normal, -q_to_p));
        float cos_q = max(0.0, dot(normal_q, q_to_p));

        vec3 flux = texture(RSM_flux, uv_q).rgb;

        shading_sum += flux * (cos_p * cos_q) / (dis * dis);
    }

    return shading_sum * (512.0 * 512.0 / float(NUM_SAMPLES)) / flux_zoom_in * fr;
    // TODO: 假定所有点的权重都相同
}