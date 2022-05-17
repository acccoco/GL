#ifndef GGX_BRDF
#define GGX_BRDF
/**
 * GGX BRDF 相关的函数
 */

#define PI 3.141592653589793
#define DELTA 0.00000000000000001


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


vec3 BRDF_GGX(float D, vec3 F, float G, vec3 n, vec3 l, vec3 v)
{
    float deno = 4.0 * max(DELTA, dot(n, l)) * max(DELTA, dot(n, v));
    return D * F * G / deno;
}

#endif
