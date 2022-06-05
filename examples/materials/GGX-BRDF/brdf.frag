#version 330 core

const float PI = 3.141592653589793;
const float DELTA = 0.00000000000000001;

include(GGX-BRDF.glsl)

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
