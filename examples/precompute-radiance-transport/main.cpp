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

const char *DIR = "precompute-radiance-transport";

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
    int transport_switcher = 1;
    std::string model_path = MODEL_BUNNY;
    std::string transport_inter_reflect = EXAMPLES + "precompute-radiance-transport/transport_inter_reflect.txt";
    std::string transport_unshadowed = EXAMPLES + "precompute-radiance-transport/transport_unshadowed.txt";
    std::string transport_shadowed = EXAMPLES + "precompute-radiance-transport/transport_shadowed.txt";
    std::vector<float> transport_SH_coeff{};
    GLuint VAO_unshadowed{};
    GLuint VAO_shadowed{};
    GLuint VAO_inter_reflect{};
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
     * 将 light 的球谐系数作为 uniform 属性传递给着色器
     */
    void set_SH_light();

    /**
     * 从文件中读取模型 transport 部分的球谐系数
     */
    std::vector<float> read_transport_sh(const std::string &file_path);

    /**
     * 从制定路径读取 .obj 文件，以及对应的 SH 参数信息。将这些信息作为顶点属性写入模型中。
     */
    GLuint init_model(const std::vector<float> &SH, const std::string &obj_file_path);

    void init() override
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

        /// light
        read_light_sh();
        set_SH_light();

        /// model
        VAO_inter_reflect = init_model(read_transport_sh(transport_inter_reflect), model_path);
        VAO_unshadowed = init_model(read_transport_sh(transport_unshadowed), model_path);
        VAO_shadowed = init_model(read_transport_sh(transport_shadowed), model_path);
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
        switch (transport_switcher) {
            case 1: glBindVertexArray(VAO_unshadowed); break;
            case 2: glBindVertexArray(VAO_shadowed); break;
            case 3: glBindVertexArray(VAO_inter_reflect); break;
            default: glBindVertexArray(VAO_unshadowed); break;
        }
        glDrawElements(GL_TRIANGLES, vertex_cnt, GL_UNSIGNED_INT, nullptr);
    }

    void tick_gui() override
    {
        ImGui::Begin("setting");
        ImGui::Text("camera pos: (%.2f, %.2f, %.2f)", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);
        ImGui::Text("camera eular: (yaw = %.2f, pitch = %.2f)", camera.get_euler().yaw, camera.get_euler().pitch);
        ImGui::SliderInt("scene switcher", &transport_switcher, 1, 3);
        ImGui::Text("1 = unshadowed");
        ImGui::Text("2 = shadowed");
        ImGui::Text("3 = inter reflect");
        ImGui::End();
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

GLuint EngineTest::init_model(const std::vector<float> &SH, const std::string &obj_file_path)
{
    const int POS = 3;
    const int NORMAL = 3;
    const int TEX = 2;
    constexpr int PNT = POS + NORMAL + TEX;
    constexpr int PNTSH = PNT + SH_MAX_CNT;

    /// just need one model
    auto obj_data = read_obj(obj_file_path)[0];
    vertex_cnt = obj_data.vertices.size() / 8;
    auto face_cnt = obj_data.faces.size() / 3;

    GLuint VAO;
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
    glBufferSubData(GL_ARRAY_BUFFER, vertex_cnt * sizeof(float) * PNT, vertex_cnt * sizeof(float) * SH_MAX_CNT, &SH[0]);

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

    /// unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return VAO;
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

std::vector<float> EngineTest::read_transport_sh(const std::string &file_path)
{
    std::fstream fs;
    fs.open(file_path, std::ios::in);
    if (!fs.is_open())
        SPDLOG_ERROR("fail to open file: {}", file_path);

    /// fs -> ss
    std::stringstream ss;
    for (std::string str; std::getline(fs, str); ss << str << " ") {}
    fs.close();

    /// lines = face cnt * 3
    int vertex_cnt_{};
    ss >> vertex_cnt_;
    auto SH = std::vector<float>(vertex_cnt_ * SH_MAX_CNT);
    for (int i = 0; i < vertex_cnt_; ++i) {
        for (int j = 0; j < SH_MAX_CNT; ++j)
            ss >> SH[i * SH_MAX_CNT + j];
    }

    fs.close();
    return SH;
}
