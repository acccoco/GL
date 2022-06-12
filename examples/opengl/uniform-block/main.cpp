/**
 * 测试 Uniform 缓冲对象和 Uniform 块布局
 */
#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/shader.h"
#include "functions/axis.h"


/// uniform block 成员的 align
struct UBMember {
    int offset;    /// 偏移：bytes
    int size;      /// 实际长度：bytes
};


/// uniform block UBSimple 的内存布局
struct {
    const UBMember m_view     = {0, 64};
    const UBMember m_proj     = {64, 64};
    const UBMember camera_pos = {128, 12};

    const int    total_size     = 144;
    const GLuint bounding_point = 1;
} UBSimple;


class ShaderDiffuse
{
    Shader2 shader = {EXAMPLE_CUR_PATH + "shader/diffuse.vert",
                      EXAMPLE_CUR_PATH + "shader/diffuse.frag"};

public:
    ShaderDiffuse()
    {
        GLuint UBSimple_index = glGetUniformBlockIndex(shader.get_program_id(), "UBSimple");
        glUniformBlockBinding(shader.get_program_id(), UBSimple_index, UBSimple.bounding_point);
    }

    void draw(const RTObject &m)
    {
        const Material &mat = m.mesh.mat;
        if (mat.has_tex_basecolor())
            glBindTexture_(GL_TEXTURE_2D, 0, mat.metallic_roughness.tex_base_color);
        shader.set_uniform({
                {"m_model", m.matrix()},
                {"has_diffuse", mat.has_tex_basecolor()},
                {"kd", glm::vec3(mat.metallic_roughness.base_color)},
                {"tex_diffuse", 0},
        });

        m.mesh.draw();
    }
};


class EngineTest : public Engine
{
    /// uniform block object
    GLuint ubo_simple{};
    GLuint ubo_mat{};

    Axis axis;

    /// model
    std::vector<RTObject> model_diona = ImportObj::load_obj(MODEL_DIONA);

    /// shader
    ShaderDiffuse shader_diffuse;

    /// 新建一个 uniform block oobject
    static GLuint create_ubo(int total_size);

    void init() override
    {
        /// 创建 uniform block 对象，并绑定到 bounding point
        ubo_simple = create_ubo(UBSimple.total_size);
        glBindBufferBase(GL_UNIFORM_BUFFER, UBSimple.bounding_point, ubo_simple);

        /// 为 uniform block UBSimple 设置初始值
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_simple);
        glBufferSubData(GL_UNIFORM_BUFFER, UBSimple.m_proj.offset, UBSimple.m_proj.size,
                        glm::value_ptr(camera.proj_matrix()));
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_simple);
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /// 设置 uniform block UBSimple
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_simple);
        glBufferSubData(GL_UNIFORM_BUFFER, UBSimple.m_view.offset, UBSimple.m_view.size,
                        glm::value_ptr(camera.view_matrix()));
        glBufferSubData(GL_UNIFORM_BUFFER, UBSimple.camera_pos.offset, UBSimple.camera_pos.size,
                        glm::value_ptr(camera.get_pos()));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        /// 绘制模型
        for (auto &m: model_diona)
            shader_diffuse.draw(m);

        axis.draw(camera.proj_matrix() * camera.view_matrix());
    }

    void tick_gui() override {}
};

GLuint EngineTest::create_ubo(int total_size)
{
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, total_size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return ubo;
}


int main()
{
    auto engine = EngineTest();
    engine.engine_main();
}