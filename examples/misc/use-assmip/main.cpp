#include "core/misc.h"
#include "core/engine.h"
#include "core/shader.h"
#include "core/import-obj.h"
#include "shader/diffuse/diffuse.h"
#include "config.hpp"


class UseAssimp : public Engine
{

    // std::vector<RTObject> obj_list = ImportObj::load_obj(MODEL_DIONA);
    Shader2 shader_diffuse = {SHADER + "diffuse/diffuse.vert", SHADER + "diffuse/diffuse.frag"};
    std::vector<RTObject> obj_list2 = ImportObj::load_obj(MODEL + "VeronicaPagan/export.obj");

    void init() override
    {
        shader_diffuse.set_uniform({
                {"m_proj", camera.proj_matrix()},
        });
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const RTObject &obj: obj_list2)
        {
            const Material &mat = obj.mesh.mat;
            if (mat.has_tex_basecolor())
                glBindTexture_(GL_TEXTURE_2D, 0, mat.metallic_roughness.tex_base_color);
            shader_diffuse.set_uniform({
                    {"m_model", obj.matrix()},
                    {"m_view", camera.view_matrix()},
                    {"kd", glm::vec3(mat.metallic_roughness.base_color)},
                    {"has_diffuse", mat.has_tex_basecolor()},
                    {"tex_diffuse", 0},
            });
            obj.mesh.draw();
        }
    }
};


int main()
{
    auto engine = UseAssimp();
    engine.engine_main();
}