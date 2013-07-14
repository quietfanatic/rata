#include <stdexcept>
#include "../../hacc/inc/everything.h"
#include "../../core/inc/phases.h"
#include "../../core/inc/opengl.h"
#include "../../util/inc/debug.h"
#include "../inc/sprites.h"

namespace vis {
    using namespace core;

    static Logger draw_sprite_logger ("draw_sprite", false);
    static Links<Sprites> sprites;

    void Sprites::appear () { link(sprites); }
    void Sprites::disappear () { unlink(); }

    struct Sprites_Renderer::Data {
        Program* program = hacc::File("modules/vis/res/sprites.prog").data().attr("prog");
        GLint tex = program->require_uniform("tex");
        GLint camera_pos = program->require_uniform("camera_pos");
        GLint model_pos = program->require_uniform("model_pos");
        GLint model_scale = program->require_uniform("model_scale");

        Data () {
            glUseProgram(program->glid);
            glUniform1i(tex, 0);  // Texture unit 0
            if (diagnose_opengl("after setting uniforms and stuff")) {
                throw std::logic_error("sprites init failed due to GL error");
            }
        }
    };

    Sprites_Renderer::Sprites_Renderer () : data(new Data) { }

    void Sprites_Renderer::run () {
        glDepthFunc(GL_LESS);
        glUseProgram(data->program->glid);
        glUniform2f(data->camera_pos, vis::camera_pos.x, vis::camera_pos.y);

        for (auto& p : sprites) {
            p.Sprites_draw(*this);
        }
    }

    void Sprites_Renderer::draw_sprite (Frame* frame, Texture* tex, Vec p, bool fliph, bool flipv, float z) {
        if (draw_sprite_logger.on) {
            draw_sprite_logger.log("tex: %s frame: [%g %g] [%g %g %g %g] p: [%g %g] fliph: %u flipv: %u, z: %g",
                tex ? tex->name.c_str() : "NULL", frame ? frame->offset.x : 0/0.0, frame ? frame->offset.y : 0/0.0,
                frame ? frame->box.l : 0/0.0, frame ? frame->box.b : 0/0.0, frame ? frame->box.r : 0/0.0, frame ? frame->box.t : 0/0.0,
                p.x, p.y, fliph, flipv, z
            );
        }

        glUniform3f(data->model_pos, p.x, p.y, z);
        glUniform2f(data->model_scale, fliph ? -1.0 : 1.0, flipv ? -1.0 : 1.0);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glBindVertexArray(frame->parent->vao_id);
        glDrawArrays(GL_QUADS, 4 * (frame - frame->parent->frames.data()), 4);

        diagnose_opengl("After rendering a sprite");
    }

    struct Sprite_Test : Sprites {
        Image* image = hacc::File("modules/vis/res/test.hacc").data().attr("img");
        Texture* texture = image->texture_named("ALL");
        Layout* layout = hacc::File("modules/vis/res/test.hacc").data().attr("layout");
        Frame* white = layout->frame_named("white");
        Frame* red = layout->frame_named("red");
        Frame* green = layout->frame_named("green");
        Frame* blue = layout->frame_named("blue");

        void finish () { appear(); }
        void Sprites_draw (Sprites_Renderer r) override {

            r.draw_sprite(white, texture, Vec(2, 2), false, false, 0);
            r.draw_sprite(red, texture, Vec(18, 2), false, false, 0);
            r.draw_sprite(green, texture, Vec(18, 13), false, false, 0);
            r.draw_sprite(blue, texture, Vec(2, 13), false, false, 0);
        }
    };

} using namespace vis;

HCB_BEGIN(Sprite_Test)
    name("vis::Sprite_Test");
    finish(&Sprite_Test::finish);
HCB_END(Sprite_Test)
