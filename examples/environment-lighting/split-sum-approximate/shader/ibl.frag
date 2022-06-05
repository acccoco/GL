#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform samplerCube filtered_env_map;
uniform float total_cube_mip_level;
uniform sampler2D brdf_lut;

uniform vec3 camera_pos;

uniform float roughness;
uniform vec3 F0;


vec3 fresnel_schlick_roughness(float NdotV, vec3 F0, float roughness);
void main()
{
    vec3 V = normalize(camera_pos - FragPos);
    vec3 N = normalize(Normal);
    vec3 R = reflect(-V, N);

    vec3 F = fresnel_schlick_roughness(max(0.0, dot(N, V)), F0, roughness);
    vec3 KS = F;            // from microsurface reflect
    vec3 KD = 1.0 - KS;     // from microsurface transmission

    // fixme
    vec3 diffuse = vec3(0.001);

    vec3 prefilter_color = textureLod(filtered_env_map, R, roughness * (total_cube_mip_level - 1.0)).rgb;
    prefilter_color = pow(prefilter_color, vec3(2.2));
    vec2 env_brdf = texture(brdf_lut, vec2(max(0.0, dot(N, V)), roughness)).rg;
    vec3 specular = prefilter_color * (F0 * env_brdf.x + env_brdf.y);

    FragColor = vec4(pow(diffuse + specular, vec3(1.0 / 2.2)), 1.0);
}

vec3 fresnel_schlick_roughness(float NdotV, vec3 F0, float roughness)
{
    return F0 + (max(F0, vec3(1.0 - roughness)) - F0) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
}