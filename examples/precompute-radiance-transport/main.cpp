/**
 * 使用球谐函数来展开「环境光照」和「传输项」
 */
#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/mesh.h"
#include "core/model.h"
#include "shader/skybox/skybox.h"

#include "./shader.h"


/// sh 只需要前 3 阶，也就是 9 个函数
const int SH_ORDER = 3;
constexpr int SH_MAX_CNT = SH_ORDER * SH_ORDER;


class EngineTest : public Engine
{
    /// light
    GLuint cubemap_indoor = load_cube_map(CUBE_INDOOR + "posx.jpg", CUBE_INDOOR + "negx.jpg", CUBE_INDOOR + "posy.jpg",
                                       CUBE_INDOOR + "negy.jpg", CUBE_INDOOR + "posz.jpg", CUBE_INDOOR + "negz.jpg");
    std::string light_SH_path = EXAMPLES + "precompute-radiance-transport/light.txt";
    std::array<std::array<float, SH_MAX_CNT>, 3> light_SH_coeff{};


    /// model path
    std::string model_path = MODEL_BUNNY;
    std::string transport_SH_path = EXAMPLES + "precompute-radiance-transport/transport_shadowed.txt";
    std::vector<float> transport_SH_coeff{};
    GLuint VAO{};
    GLsizei vertex_cnt{};

    /// shader
    ShaderPRT shader_ptr;
    CubeMapVisual cube_visual;

    /**
     * 从文件中读取环境光照的球谐系数\n
     * 格式为：\n
     *  R G B \n
     *  R G B \n
     *  ...\n
     * 只记录前 3 阶的球谐函数
     */
    void read_light_sh();

    /**
     * 从文件中读取模型 transport 部分的球谐系数
     */
    void read_transport_sh();

    /**
     * 将 light 的球谐系数作为 uniform 属性传递给着色器
     */
    void set_SH_light();

    /**
     * 从制定路径读取 .obj 文件，以及对应的 SH 参数信息。将这些信息作为顶点属性写入模型中。
     */
    void init_model();

    void init() override
    {
        read_light_sh();
        read_transport_sh();
        set_SH_light();
        init_model();

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /// draw skybox
        cube_visual.draw_as_skybox(camera.view_matrix(), Camera::proj_matrix(), cubemap_indoor);

        /// draw bunny
        shader_ptr.m_proj.set(Camera::proj_matrix());
        shader_ptr.m_view.set(camera.view_matrix());
        shader_ptr.m_model.set(glm::one<glm::mat4>());
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, vertex_cnt, GL_UNSIGNED_INT, nullptr);
    }
};

int main()
{
    auto engine = EngineTest();
    engine.engine_main();
}

void EngineTest::set_SH_light()
{
    shader_ptr.use();
    glUniformMatrix3fv(shader_ptr.SH_light_R.location, 1, GL_FALSE, &light_SH_coeff[0][0]);
    glUniformMatrix3fv(shader_ptr.SH_light_G.location, 1, GL_FALSE, &light_SH_coeff[1][0]);
    glUniformMatrix3fv(shader_ptr.SH_light_B.location, 1, GL_FALSE, &light_SH_coeff[2][0]);
}

void EngineTest::init_model()
{
    const int POS = 3;
    const int NORMAL = 3;
    const int TEX = 2;
    constexpr int PNT = POS + NORMAL + TEX;
    constexpr int PNTSH = PNT + SH_MAX_CNT;

    /// just need one model
    auto obj_data = read_obj(model_path)[0];
    vertex_cnt = obj_data.vertices.size() / 8;
    auto face_cnt = obj_data.faces.size() / 3;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /**
     * 这里使用 buffer sub data
     */
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, PNTSH * vertex_cnt * sizeof(float), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_cnt * sizeof(float) * PNT, &obj_data.vertices[0]);
    glBufferSubData(GL_ARRAY_BUFFER, vertex_cnt * sizeof(float) * PNT, vertex_cnt * sizeof(float) * SH_MAX_CNT,
                    &transport_SH_coeff[0]);

    /**
     * 注意：每个顶点属性的最大尺寸是 4，因此 9 个 SH 参数需要分三次传入
     */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, POS, GL_FLOAT, GL_FALSE, PNT * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, NORMAL, GL_FLOAT, GL_FALSE, PNT * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, TEX, GL_FLOAT, GL_FALSE, PNT * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, SH_MAX_CNT * sizeof(float),
                          (void *) (vertex_cnt * 8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, SH_MAX_CNT * sizeof(float),
                          (void *) (vertex_cnt * 8 * sizeof(float) + 3 * sizeof(float)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, SH_MAX_CNT * sizeof(float),
                          (void *) (vertex_cnt * 8 * sizeof(float) + 6 * sizeof(float)));

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_cnt * 3 * sizeof(unsigned int), &obj_data.faces[0], GL_STATIC_DRAW);
}

void EngineTest::read_light_sh()
{
    /// read file
    std::fstream fs;
    fs.open(light_SH_path, std::ios::in);
    if (!fs.is_open())
        SPDLOG_ERROR("fail to open file: {}", light_SH_path);

    /// file -> stream
    std::stringstream ss;
    {
        int lines = 0;
        for (std::string str; std::getline(fs, str); ss << str << " ")
            ++lines;
        fs.close();
        if (lines != SH_MAX_CNT)
            SPDLOG_ERROR("SH light order error, expect {}, but: {}", SH_MAX_CNT, lines);
    }


    for (int i = 0; i < SH_MAX_CNT; ++i) {
        for (int j = 0; j < 3; ++j) {/// 3 = RGB
            ss >> light_SH_coeff[j][i];
        }
    }
}

void EngineTest::read_transport_sh()
{
    std::fstream fs;
    fs.open(transport_SH_path, std::ios::in);
    if (!fs.is_open())
        SPDLOG_ERROR("fail to open file: {}", transport_SH_path);

    /// fs -> ss
    std::stringstream ss;
    for (std::string str; std::getline(fs, str); ss << str << " ") {}
    fs.close();

    /// lines = face cnt * 3
    int vertex_cnt_{};
    ss >> vertex_cnt_;
    transport_SH_coeff = std::vector<float>(vertex_cnt_ * SH_MAX_CNT);
    for (int i = 0; i < vertex_cnt_; ++i) {
        for (int j = 0; j < SH_MAX_CNT; ++j)
            ss >> transport_SH_coeff[i * SH_MAX_CNT + j];
    }

    fs.close();
}
