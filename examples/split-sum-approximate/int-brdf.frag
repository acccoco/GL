#version 330 core

in vec2 TexCoord;
out vec2 FragColor;

const uint SAMPLE_COUNT = 1024u;
const float PI = 3.141592653589793238462643383279502884;


vec2 Hammersley(uint i, uint N);
vec3 importance_sample_GGX(vec2 Xi, float roughness, vec3 N);
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness);
// integrate in tangent space
vec2 int_BRDF(float NdotV, float roughness)
{
    // in tangent space
    vec3 N = vec3(0.0, 0.0, 1.0);
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);

    // integrate part
    float A = 0.0;
    float B = 0.0;

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = importance_sample_GGX(Xi, roughness, N);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(0.0, L.z);// notice: N = (0, 0, 1)
        float NdotH = max(0.0, H.z);
        float VdotH = max(0.0, dot(V, H));

        if (NdotL > 0.0)
        {
            float G = geometry_smith(N, V, L, roughness);
            float G_vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);// from fresnel

            A += (1.0 - Fc) * G_vis;
            B += Fc * G_vis;
        }
    }
    return vec2(A, B) / float(SAMPLE_COUNT);
}

void main() {
    FragColor = int_BRDF(TexCoord.x, TexCoord.y);
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
    float a = roughness * roughness;// better visual effect

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

float geometry_schlick_GGX(float NdotV, float roughness)
{
    float k = (roughness * roughness) / 2.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(0.0, dot(N, V));
    float NdotL = max(0.0, dot(N, L));

    return geometry_schlick_GGX(NdotL, roughness) * geometry_schlick_GGX(NdotV, roughness);
}
