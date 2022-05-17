#version 330 core

const float PI = 3.141592653589793;
const float DELTA = 0.00000000000000001;

in VS_FS
{
    vec3 Pos;
    vec3 Normal;
    vec2 UV;
} vs_fs;

out vec4 FragColor;

uniform vec3 camera_pos;
uniform vec3 light_pos;

uniform vec3 light_color;
uniform vec3 ambient_light;

uniform sampler2D tex_roughness;
uniform sampler2D tex_albedo;


/// Fresnel 项，确保是单位向量
vec3 Fresnel(vec3 F0, vec3 v, vec3 h);

/// shadow masking 项，确保是单位向量
float Geometry_Smith(vec3 n, vec3 v, vec3 l, float alpha);

/// 法线分布函数，确保是单位向量
float NDF(vec3 n, vec3 h, float alpha);

/// GGX 的 BRDF，确保是单位向量
vec3 BRDF_GGX(float D, vec3 F, float G, vec3 n, vec3 l, vec3 v);

/// 点光源下的着色
vec3 punctual_light_shading(vec3 n, vec3 l, vec3 brdf, vec3 c_light);

void main()
{
    vec3 n = normalize(vs_fs.Normal);
    vec3 l = normalize(light_pos - vs_fs.Pos);
    vec3 v = normalize(camera_pos - vs_fs.Pos);
    vec3 h = normalize((l + v) * 0.5);

    /// Fresnel
    vec3 albedo = texture(tex_albedo, vs_fs.UV).rgb;
    vec3 F = Fresnel(albedo, v, h);

    /// specular
    float roughness = texture(tex_roughness, vs_fs.UV).r;
    float D = NDF(n, h, roughness);
    float G = Geometry_Smith(n, v, l, roughness);
    vec3 brdf_specular = BRDF_GGX(D, F, G, n, l, v);
    vec3 specular = punctual_light_shading(n, l, brdf_specular, light_color);

    /// ambient
    vec3 ambient = albedo * ambient_light;

    /// final color 
    FragColor = vec4(specular + ambient, 1.0);
}


vec3 punctual_light_shading(vec3 n, vec3 l, vec3 brdf, vec3 c_light)
{
    return PI * brdf * c_light * max(0.0, dot(n, l));
}


vec3 BRDF_GGX(float D, vec3 F, float G, vec3 n, vec3 l, vec3 v)
{
    float deno = 4.0 * max(DELTA, dot(n, l)) * max(DELTA, dot(n, v));
    return D * F * G / deno;
}


float NDF(vec3 n, vec3 h, float alpha)
{
    float alpha2 = alpha * alpha;
    float n_dot_h = max(DELTA, dot(n, h));
    float deno = PI * pow(n_dot_h * n_dot_h * (alpha2 - 1.0) + 1.0, 2.0);
    return alpha2 / deno;
}

vec3 Fresnel(vec3 F0, vec3 v, vec3 h)
{
    return F0 + (1 - F0) * pow((1 - max(0.0, dot(v, h))), 5.0);
}


float Geometry_Schlick(vec3 n, vec3 dir, float alpha)
{
    float n_dot_v = max(DELTA, dot(n, dir));
    float k = alpha / 2.0;
    float deno = n_dot_v * (1.0 - k) + k;
    return n_dot_v / deno;
}


float Geometry_Smith(vec3 n, vec3 v, vec3 l, float alpha)
{
    return Geometry_Schlick(n, v, alpha) * Geometry_Schlick(n, l, alpha);
}

