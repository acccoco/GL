#version 330 core

const int   MAX_STEPS = 100;        // ray march 最多走的步数
const float DIS_INF   = 1000;
const float DIS_DELTA = 1e-3;       // ray march 的最小步长
const float PI        = 3.141592653589793;

in vec2  TexCoord;
out vec4 FragColor;

uniform vec3  light_pos;
uniform vec3  light_color;
uniform float camera_fov;
uniform vec3  camera_pos;
uniform vec3  camera_front;
uniform vec3  camera_up;
uniform vec3  camera_right;     // 这三者互相垂直
uniform float near;             // 摄像机近平面的位置

/// 球的 SDF
float sdf_sphere(vec3 center, float radius, vec3 p)
{
    return length(p - center) - radius;
}

/// 长方体的 SDF 
/// @param size 就是世界坐标系下 xyz 三个方向的尺寸
float sdf_box(vec3 size, vec3 p)
{
    vec3 q = abs(p) - size;
    return length(max(vec3(0.0), q)) + min(0.0, max(q.x, max(q.y, q.z)));
}

/// 整个场景的 SDF
float sdf_scene(vec3 p)
{
    /// 球体
    float dis_sphere = sdf_sphere(vec3(0.0, 1.0, 0.0), 1.0, p);

    /// 地板：长方体
    float dis_floor = sdf_box(vec3(5.0, 0.1, 5.0), p);


    return min(dis_sphere, dis_floor);
}

/// 计算某个点（接近表面）的法线
vec3 get_sdf_normal(vec3 p)
{
    const float DELTA = 1e-3;
    const float DELTA_INVERSE = 1.0 / DELTA;
    vec3 n;
    float p_dis = sdf_scene(p);
    vec3 temp_p;
    for (int i = 0; i < 3; ++i)
    {
        temp_p = p;
        temp_p[i] += DELTA;
        n[i] = (sdf_scene(temp_p) - p_dis) * DELTA_INVERSE;
    }
    return normalize(n);
}

/// 进行 ray march
/// @param ray_dir 确保 ray_dir 是单位向量
/// @return 返回 ray 走过的距离
float ray_march(vec3 ray_origin, vec3 ray_dir)
{
    float ray_dis = 0.0;    // ray 相比于出发点，前进了多少距离
    int steps = 0;          // 走过的步数
    while (steps < MAX_STEPS && ray_dis < DIS_INF)
    {
        float dis = sdf_scene(ray_origin + ray_dir * ray_dis);
        ray_dis += dis;
        if (dis < DIS_DELTA) break;
        ++steps;
    }
    return ray_dis;
}

vec3 shading(vec3 pos, vec3 l_pos)
{
    vec3 n = get_sdf_normal(pos);
    vec3 l = normalize(l_pos - pos);
    vec3 v = normalize(camera_pos - pos);
    vec3 h = normalize(v + l);

    /// ambient
    float ka = 0.1;
    vec3 ambient = ka * light_color;

    /// diffuse
    float kd = 0.6;
    vec3 diffuse = kd * max(0.0, dot(l, n)) * light_color;

    /// specular
    float ks = 0.3;
    vec3 specular = ks * light_color * pow(max(0.0, dot(n, h)), 32);

    /// 超光源进行 ray march，看能够走多远
    /// + n * 1e-5 是为了避免自相交 
    float to_light = ray_march(pos + n * 2e-3, l);
    float visibility = 1.0;
    if (to_light < length(l_pos - pos)) {
        visibility = 0.35;
    }

    return (ambient + diffuse + specular) * visibility;
}

void main() 
{
    /// 将纹理坐标从 [0, 1]^2 转换为 [-0.5, 0.5]^2
    vec2 canvas_coord = TexCoord - 0.5;
    float canvas_size = 2.0 * near * tan(camera_fov / 2.0 / 180.0 * PI);

    /// 计算光线的方向：从摄像机朝近平面像素上发射光线
    vec3 ray_dir = camera_front * near;
    ray_dir += canvas_size * canvas_coord.x * camera_right;
    ray_dir += canvas_size * canvas_coord.y * camera_up;
    ray_dir = normalize(ray_dir);

    /// 计算光线和场景的交点
    /// todo：简单假设，如果距离大于 100，就视为没有相交
    float ray_dis = ray_march(camera_pos, ray_dir);
    if (ray_dis > 100.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    vec3 inter_pos = camera_pos + ray_dir * ray_dis;
    FragColor = vec4(shading(inter_pos, light_pos), 1.0);
}