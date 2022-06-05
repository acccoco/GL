#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform bool has_diffuse;
uniform sampler2D tex_diffuse;
uniform vec3 kd;
uniform vec3 ks;
uniform vec3 camera_pos;
uniform vec3 light_pos;
uniform samplerCube shadow_map_cube;

const float light_indensity = 2.0f;

vec3 shading()
{
    // color from texture, gamma correct
    vec3 color = has_diffuse ? pow(texture(tex_diffuse, TexCoord).rgb, vec3(2.2)) : kd;

    // ambient
    vec3 ambient = 0.15 * color;

    // diffuse
    vec3 light_dir = normalize(FragPos - light_pos);
    vec3 normal = normalize(Normal);
    float diff = max(dot(-light_dir, normal), 0.f);
    float light_atten_coff = light_indensity / (1 + length(light_pos - FragPos));
    vec3 diffuse = diff * light_atten_coff * color;
    // vec3 diffuse = abs(light_dir);   // fantastic color

    // specular
    vec3 view_dir = normalize(FragPos - camera_pos);
    vec3 reflect_dir = reflect(light_dir, normal);
    float spec = pow(max(dot(-view_dir, reflect_dir), 0.f), 25.f);
    vec3 specular = ks * light_atten_coff * spec;

    return ambient + diffuse + specular;
}

float visibility()
{
    const float near = 0.1;
    const float far = 20;
    vec3 light_dir = normalize(FragPos - light_pos);
    float blocker_depth = texture(shadow_map_cube, light_dir).r;
    float frag_depth = length(FragPos - light_pos);
    float bias = 0.005;
    return frag_depth - bias > blocker_depth ? 0.5 : 1;
}

void main() {
    FragColor = vec4(pow(shading() * visibility(), vec3(1.0/2.2)), 1.0);
}