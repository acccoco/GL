#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 frag_clip_light_coord;

out vec4 FragColor;

uniform sampler2D shadow_map;
uniform vec3 light_pos;
uniform vec3 camera_pos;
uniform vec3 kd;
uniform vec3 ks;
uniform bool has_diffuse;
uniform sampler2D tex_diffuse;
uniform vec3 rand_seed;

const float BIAS_BASE = 0.1f;
const float BIAS_MIN = 0.1f;
const float light_indensity = 2.0f;
const float light_size = 0.5f;      // assume light size is 0.5
const float near = 0.1;
const float near_width = 0.2;

const float PI = 3.141592653589793;
const float _2PI = 6.283185307179586;
const int NUM_RINGS = 7;
const int NUM_SAMPLES = 16;

vec2 poisson_disk[NUM_SAMPLES];

/**
 1. 可以选择通过 gen_possion_disk 来动态生成泊松圆盘，也可以直接使用静态的泊松圆盘
 2. 可以选择三种阴影方式：shadow mapping, pcf, pcss
 3. 可以手动调节 bias 相关的参数
 参考：https://developer.download.nvidia.cn/whitepapers/2008/PCSS_Integration.pdf
 */

float rand2(vec2 xy)
{
    float dt = dot(xy, vec2(12.9898, 78.233));
    float sn = mod(dt, PI);
    return fract(sin(sn) * 43758.5453);
}

vec2 static_poisson[16] = vec2[](
    vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ),
    vec2( -0.094184101, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ),
    vec2( -0.91588581, 0.45771432 ),
    vec2( -0.81544232, -0.87912464 ),
    vec2( -0.38277543, 0.27676845 ),
    vec2( 0.97484398, 0.75648379 ),
    vec2( 0.44323325, -0.97511554 ),
    vec2( 0.53742981, -0.47373420 ),
    vec2( -0.26496911, -0.41893023 ),
    vec2( 0.79197514, 0.19090188 ),
    vec2( -0.24188840, 0.99706507 ),
    vec2( -0.81409955, 0.91437590 ),
    vec2( 0.19984126, 0.78641367 ),
    vec2( 0.14383161, -0.14100790 )
);

// generate posisson disk
void gen_poisson_disk(vec2 rand_seed)
{
    const float ANGLE_STEP = _2PI * float(NUM_RINGS) / float(NUM_SAMPLES);

    float angle = rand2(rand_seed) * _2PI;      // init angle
    float radius= 1.0 / float(NUM_SAMPLES);     // init radius and radius step
    float radius_step = radius;

    for (int i = 0; i < NUM_SAMPLES; ++i) {
        poisson_disk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
        radius += radius_step;
        angle += ANGLE_STEP;
    }
}


vec3 g_ambient;
vec3 g_diffuse;
vec3 g_specular;
void phong_shading()
{
    vec3 color = has_diffuse ? pow(texture(tex_diffuse, TexCoord).rgb, vec3(2.2)) : kd;

    // ambient
    g_ambient = 0.20 * color;

    // diffuse
    vec3 light_dir = normalize(FragPos - light_pos);
    vec3 normal = normalize(Normal);
    float diff = max(dot(-light_dir, normal), 0.f);
    float light_atten_coff = light_indensity / (1 + length(light_pos - FragPos));
    g_diffuse = diff * light_atten_coff * color;

    // specular
    vec3 view_dir = normalize(FragPos - camera_pos);
    vec3 reflect_dir = reflect(light_dir, normal);
    float spec = pow(max(dot(-view_dir, reflect_dir), 0.f), 25.f);
    g_specular = ks * light_atten_coff * spec;
}

float visibility_shadow_mapping(float frag_depth, vec2 frag_uv, float bias)
{
    float blocker_depth = texture(shadow_map, frag_uv).r;

    return frag_depth - bias > blocker_depth ? 0.5 : 1;
}

float visibility_pcf(float frag_depth, vec2 frag_uv, float bias)
{
    float visible_sum = 0.f;
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        float blocker_depth = texture(shadow_map, frag_uv + static_poisson[i] / 200.f).r;
        visible_sum += frag_depth - bias > blocker_depth ? 0.5f : 1.f;
    }

    return visible_sum / float(NUM_SAMPLES);
}


float visibility_pcss(float frag_depth, vec2 frag_uv, float bias)
{
    // first step: blocker search
    float blocker_search_radius = (frag_depth - near) / frag_depth * light_size / near_width;
    float blocker_depth = 0.f;
    int blocker_cnt = 0;
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        float depth = texture(shadow_map, frag_uv + poisson_disk[i] * blocker_search_radius).r;
        if (frag_depth - bias < depth) continue;
        ++blocker_cnt;
        blocker_depth += depth;
    }
    if (blocker_cnt == 0) return 1.0f;
    blocker_depth /= float(blocker_cnt);

    // second step: determin filter size
    float penumbra = (frag_depth - blocker_depth) * light_size / blocker_depth;

    // third step: pcf
    float visibility = 0.f;
    float filter_radius = penumbra * light_size / near_width * near / frag_depth;
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        float blocker_depth = texture(shadow_map, frag_uv + poisson_disk[i] * filter_radius / 10.0).r;
        visibility += (frag_depth - bias < blocker_depth) ? 1.0f : 0.0;
    }

    return visibility / float(NUM_SAMPLES);
}


void main()
{
    // generate poisson disk
    gen_poisson_disk(rand_seed.xy);

    float frag_depth = frag_clip_light_coord.w;
    vec2 frag_uv = frag_clip_light_coord.xy / frag_clip_light_coord.w * 0.5 + 0.5;

    float tan_theta = tan(acos(clamp(dot(normalize(Normal), normalize(light_pos - FragPos)), 0, 1)));
    float bias = max(BIAS_MIN, BIAS_BASE * tan_theta);

    phong_shading();

    // three implement: visibility_pcss, visibility_shadow_mapping, visibility_pcf
    FragColor = vec4(pow((g_diffuse + g_specular) * visibility_pcss(frag_depth, frag_uv, bias) + g_ambient, vec3(1.0/2.2)), 1.0);
}
