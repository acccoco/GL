#version 330 core

const float PI = 3.141592653589793238462643383279502884;
const uint SAMPLE_COUNT = 1024u;

in vec3 TexCoord3;

out vec4 FragColor;

// env-map color is RGB, not sRGB
uniform samplerCube env_map;
uniform float roughness;

vec2 Hammersley(uint i, uint N);
vec3 importance_sample_GGX(vec2 Xi, float roughness, vec3 N);
void main()
{
    vec3 N = normalize(TexCoord3);
    vec3 R = N;
    vec3 V = R;

    vec3 color = vec3(0.0);
    float weight = 0.0;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = importance_sample_GGX(Xi, roughness, N);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(0.0, dot(N, L));
        if (NdotL > 0.0)
        {
            color += pow(texture(env_map, L).rgb, vec3(2.2)) * NdotL;
            weight += NdotL;
        }
    }

    FragColor = vec4(pow(color / weight, vec3(1.0/2.2)), 1.0);
}

// radical inverse van der corput
float radical_inverse_VDC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;// / 0x100000000
}

// 低差异序列
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), radical_inverse_VDC(i));
}

// GGX 采样
vec3 importance_sample_GGX(vec2 Xi, float roughness, vec3 N)
{
    float a = roughness * roughness;    // better visual effect

    // uniform distribution to GGX distribution
    float phi = 2 * PI * Xi.x;
    float cos_theta = sqrt((1.0 - Xi.y) / (1 + (a * a - 1) * Xi.y));
    float sin_theta = sqrt(1 - cos_theta * cos_theta);

    // half-way vector in tagent space
    vec3 H;
    H.x = sin_theta * cos(phi);
    H.y = sin_theta * sin(phi);
    H.z = cos_theta;

    // tangent space base vector
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    // tangent space to world space
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}
