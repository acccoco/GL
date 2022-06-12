#pragma once


// TODO 按照 gltf2.0 的标准来做，例如：basecolor 要求是 sRGB 的，之后按照 basecolor-factor 加权
struct Material {
    std::string name;

    bool double_side{false};

    struct {
        double    metallic{0.f};
        double    roughness{0.8f};
        glm::vec4 base_color{0.6f};

        int tex_metallic_roughness{-1};
        int tex_base_color{-1};
    } metallic_roughness;

    // normal = normalize((<tex value> * 2.0 - 1.0) * vec3(<scale>, <scale>, 1.0))
    double normal_scale{1.0};
    int    tex_normal{-1};

    // occludedColor = lerp(color, color * <tex value>, <occlusion strength>)
    double occusion_strength{0.0};
    int    tex_occlusion{-1};

    glm::vec3 emissive{0.f};
    int       tex_emissive{-1};

    [[nodiscard]] bool has_tex_basecolor() const { return metallic_roughness.tex_base_color != -1; }
    [[nodiscard]] bool has_tex_metallic_roughness() const
    {
        return metallic_roughness.tex_metallic_roughness != -1;
    }
    [[nodiscard]] bool has_tex_occlusion() const { return tex_occlusion != -1; }
    [[nodiscard]] bool has_tex_normal() const { return tex_normal != -1; }
    [[nodiscard]] bool has_tex_emissive() const { return tex_emissive != -1; }
};