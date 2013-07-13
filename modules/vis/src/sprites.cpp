#include <stdexcept>
#include "../../hacc/inc/everything.h"
#include "../../core/inc/phases.h"
#include "../../core/inc/opengl.h"
#include "../../util/inc/debug.h"
#include "../inc/sprites.h"

namespace vis {

    using namespace core;

    static Links<Sprite> sprites;

    void Sprite::appear () {
        link(sprites);
    }
    void Sprite::disappear () {
        unlink();
    }

    struct Sprite_Layer : Layer, Renderer {
        Program* program;
        GLint tex;
        GLint camera_pos;
        GLint model_pos;
        GLint model_scale;

        Sprite_Layer () : Layer("C.M", "sprites") { }
        void Layer_start () override {
            program = hacc::File("modules/vis/res/sprite.prog").data().attr("prog");
            tex = program->require_uniform("tex");
            camera_pos = program->require_uniform("camera_pos");
            model_pos = program->require_uniform("model_pos");
            model_scale = program->require_uniform("model_scale");
            glUseProgram(program->glid);
            glUniform1i(tex, 0);  // Texture unit 0
            if (diagnose_opengl("after setting uniforms and stuff")) {
                throw std::logic_error("sprites init failed due to GL error");
            }
        }
         // Renderer
        void start_rendering () override {
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST); // Depth buffer is awesome
            glUseProgram(program->glid);
            glUniform2f(camera_pos, vis::camera_pos.x, vis::camera_pos.y);
        }
         // Layer
        void Layer_run () override {
            glClearColor(0.5, 0.5, 0.5, 0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            for (auto p = sprites.first(); p; p = p->next()) {
                p->Sprite_draw();
            }
        }
    } sprite_layer;

    static Logger draw_sprite_logger ("draw_sprite", false);

    void draw_sprite (Frame* frame, Texture* tex, Vec p, bool fliph, bool flipv, float z) {
        if (draw_sprite_logger.on) {
            draw_sprite_logger.log("tex: %s frame: [%g %g] [%g %g %g %g] p: [%g %g] fliph: %u flipv: %u, z: %g",
                tex ? tex->name.c_str() : "NULL", frame ? frame->offset.x : 0/0.0, frame ? frame->offset.y : 0/0.0,
                frame ? frame->box.l : 0/0.0, frame ? frame->box.b : 0/0.0, frame ? frame->box.r : 0/0.0, frame ? frame->box.t : 0/0.0,
                p.x, p.y, fliph, flipv, z
            );
        }

        sprite_layer.use();

        glUniform3f(sprite_layer.model_pos, p.x, p.y, z);
        glUniform2f(sprite_layer.model_scale, fliph ? -1.0 : 1.0, flipv ? -1.0 : 1.0);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glBindVertexArray(frame->parent->vao_id);
        glDrawArrays(GL_QUADS, 4 * (frame - frame->parent->frames.data()), 4);

        diagnose_opengl("After rendering a sprite");
    }

    struct Sprite_Test : Sprite {
        void finish () { appear(); }
        void Sprite_draw () override {
            static Image* image = hacc::File("modules/vis/res/test.hacc").data().attr("img");
            static Texture* texture = image->texture_named("ALL");
            static Layout* layout = hacc::File("modules/vis/res/test.hacc").data().attr("layout");
            static Frame* white = layout->frame_named("white");
            static Frame* red = layout->frame_named("red");
            static Frame* green = layout->frame_named("green");
            static Frame* blue = layout->frame_named("blue");

            draw_sprite(white, texture, Vec(2, 2), false, false, 0);
            draw_sprite(red, texture, Vec(18, 2), false, false, 0);
            draw_sprite(green, texture, Vec(18, 13), false, false, 0);
            draw_sprite(blue, texture, Vec(2, 13), false, false, 0);
        }
    };

} using namespace vis;

HCB_BEGIN(Sprite_Test)
    name("vis::Sprite_Test");
    finish(&Sprite_Test::finish);
HCB_END(Sprite_Test)
