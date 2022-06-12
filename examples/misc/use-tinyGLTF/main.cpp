#include "tiny_gltf.h"
#include "core/misc.h"
#include "core/engine.h"
#include "config.hpp"
#include "core/shader.h"
#include "shader/diffuse/diffuse.h"
#include "core/import-gltf.h"


class TestGLTF : public Engine
{
    std::vector<RTObject> obj_list;

    Shader2 shader_base = {SHADER + "base/base.vert", SHADER + "base/base.frag"};
    Shader2 shader_gltf = {SHADER + "gltf/gltf.vert", SHADER + "gltf/gltf.frag"};

    void init() override
    {

        //        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        //        CHECK_GL_ERROR();

        // ImportGLTF gltf = ImportGLTF{MODEL + "dancing-girl/scene.gltf"};
        // GLTF gltf = GLTF {EXAMPLE_CUR_PATH + "test-gltf/test.gltf"};
        ImportGLTF gltf = ImportGLTF{"/Users/qizhengjie/Library/Mobile Documents/com~apple~CloudDocs/常用3D模型/warrior_girl/scene.gltf"};
        obj_list = gltf.get_obj_list();
    }

    void tick_render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_base.set_uniform({
                {"u_camera_pos", camera.get_pos()},
                {"u_vp", camera.proj_matrix() * camera.view_matrix()},
        });

        shader_gltf.set_uniform({
                {"u_proj", camera.proj_matrix()},

                {"u_light_pos", glm::vec3(2.f, 4.f, 2.f)},
                {"u_light_color", glm::vec3(0.7f)},
        });

        for (const auto &obj: obj_list)
        {

            const auto &mesh = obj.mesh;
            const auto &mat  = mesh.mat;
            // shader_base.set_uniform({{"u_model", obj.matrix}});

            shader_gltf.set_uniform({
                    {"u_model_view", camera.view_matrix() * obj.matrix()},

                    {"u_basecolor", mat.metallic_roughness.base_color},
                    {"u_has_basecolor", (int) mat.has_tex_basecolor()},
                    {"u_tex_basecolor", 0},

                    {"u_occlusion_strength", (float) mat.occusion_strength},
                    {"u_has_occlusion", (int) mat.has_tex_occlusion()},
                    {"u_tex_occlusion", 1},

                    {"u_emissive", mat.emissive},
                    {"u_has_emissive", (int) mat.has_tex_emissive()},
                    {"u_tex_emissive", 2},

                    {"u_normal_scale", (float) mat.normal_scale},
                    {"u_has_normal", (int) mat.has_tex_normal()},
                    {"u_tex_normal", 3},
            });
            if (mat.has_tex_basecolor())
                glBindTexture_(GL_TEXTURE_2D, 0, mat.metallic_roughness.tex_base_color);
            if (mat.has_tex_occlusion())
                glBindTexture_(GL_TEXTURE_2D, 1, mat.tex_occlusion);
            if (mat.has_tex_emissive())
                glBindTexture_(GL_TEXTURE_2D, 2, mat.tex_emissive);
            if (mat.has_tex_normal())
                glBindTexture_(GL_TEXTURE_2D, 3, mat.tex_normal);

            glBindVertexArray(mesh.vao);
            glDrawElements(mesh.primitive_mode, (GLsizei) mesh.index_cnt,
                           mesh.index_component_type, (void *) mesh.index_offset);
            CHECK_GL_ERROR();
        }
    }
};


int main()
{
    try
    {
        auto engine = TestGLTF();
        engine.engine_main();
    } catch (std::exception &ex)
    {}
}
