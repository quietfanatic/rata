#include <stdexcept>
#include <GL/gl.h>
#include <GL/glfw.h>
#include <SOIL/SOIL.h>
#include "../../hacc/inc/everything.h"
#include "../inc/sprites.h"
#include "../../core/inc/game.h"
#include "../../util/inc/debug.h"
#include "../inc/shaders.h"

namespace vis {

    static GLenum diagnose_opengl (const char* when = "") {
        GLenum err = glGetError();
        if (err)
            fprintf(stderr, "OpenGL error: %04x %s\n", err, when);
        return err;
    }

    static Logger draw_sprite_logger ("draw_sprite", false);
    static Program* sprite_program = NULL;

    void draw_sprite (Frame* frame, Texture* tex, Vec p, bool fliph, bool flipv, float z) {
        if (draw_sprite_logger.on) {
            draw_sprite_logger.log("img: %s frame: [%g %g] [%g %g %g %g] p: [%g %g] fliph: %u flipv: %u, z: %g",
                tex ? tex->name.c_str() : "NULL", frame ? frame->offset.x : 0/0.0, frame ? frame->offset.y : 0/0.0,
                frame ? frame->box.l : 0/0.0, frame ? frame->box.b : 0/0.0, frame ? frame->box.r : 0/0.0, frame ? frame->box.t : 0/0.0,
                p.x, p.y, fliph, flipv, z
            );
        }
        if (!tex || !frame) return;
        float tl, tb, tr, tt, vl, vb, vr, vt;
        if (fliph) {
            tl = frame->offset.x - frame->box.r;
            tr = frame->offset.x - frame->box.l;
            vl = p.x - frame->box.r * PX;
            vr = p.x - frame->box.l * PX;
        }
        else {
            tl = frame->offset.x + frame->box.l;
            tr = frame->offset.x + frame->box.r;
            vl = p.x + frame->box.l * PX;
            vr = p.x + frame->box.r * PX;
        }
        if (flipv) {
            tb = frame->offset.y - frame->box.t;
            tt = frame->offset.y - frame->box.b;
            vb = p.y - frame->box.t * PX;
            vt = p.y - frame->box.b * PX;
        }
        else {
            tb = frame->offset.y + frame->box.b;
            tt = frame->offset.y + frame->box.t;
            vb = p.y + frame->box.b * PX;
            vt = p.y + frame->box.t * PX;
        }
        tl /= tex->size.x;
        tb /= tex->size.y;
        tr /= tex->size.x;
        tt /= tex->size.y;
        if (draw_sprite_logger.on) {
            draw_sprite_logger.log("tex: [%g %g %g %g] vert [%g %g %g %g]",
                tl, tb, tr, tt, vl, vb, vr, vt
            );
        }
        sprite_program->use();
        glBindTexture(GL_TEXTURE_2D, tex->tex);
         // Direct Mode is still the easiest for drawing individual images.
        glBegin(GL_QUADS);
            glTexCoord2f(tl, 1-tb); glVertex3f(vl, vb, z);
            glTexCoord2f(tr, 1-tb); glVertex3f(vr, vb, z);
            glTexCoord2f(tr, 1-tt); glVertex3f(vr, vt, z);
            glTexCoord2f(tl, 1-tt); glVertex3f(vl, vt, z);
        glEnd();
    }

    struct Camera_Setup_Layer : core::Layer {
        Camera_Setup_Layer () : core::Layer("B.M", "camera_setup") { }
        void run () {
            glClearColor(0.5, 0.5, 0.5, 0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glTranslatef(-1, -1, 0);  // 0,0 starts out in the center
            glScalef(1/10.0, 1/7.5, 1);  // This is independent of window scale

             // Make coordinates point to pixels, not the corners between pixels
            glTranslatef(0.45*PX/2, 0.45*PX/2, 0);

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST); // Depth buffer is awesome
        }
    } csl;

    struct Test_Layer : core::Layer {
        Test_Layer () : core::Layer("C.M", "test") { }
        void run () {
            static Image* image = hacc::reference_file<Image>("modules/vis/res/test.image");
            static Texture* texture = image->texture_named("ALL");
            static Layout* layout = hacc::reference_file<Layout>("modules/vis/res/test.layout");
            static Frame* white = layout->frame_named("white");
            static Frame* red = layout->frame_named("red");
            static Frame* green = layout->frame_named("green");
            static Frame* blue = layout->frame_named("blue");

            draw_sprite(white, texture, Vec(2, 2), false, false, 0);
            draw_sprite(red, texture, Vec(18, 2), false, false, 0);
            draw_sprite(green, texture, Vec(18, 13), false, false, 0);
            draw_sprite(blue, texture, Vec(2, 13), false, false, 0);
        }
    } test_layer;

    static Links<Draws_Sprites> sprite_drawers;

    void Draws_Sprites::appear () {
        link(sprite_drawers);
    }
    void Draws_Sprites::disappear () {
        unlink();
    }


    struct Sprite_Layer : core::Layer {
        Sprite_Layer () : core::Layer("D.M", "sprites") { }
        void run () {
            for (Draws_Sprites* p = sprite_drawers.first(); p; p = p->next()) {
                p->draw();
            }
        }
        void init () {
            static auto glUniform1i = glproc<void (GLint, GLint)>("glUniform1i");
            sprite_program = hacc::require_id<Program>("modules/vis/res/sprite.prog");
            sprite_program->use();
            int tex_uni = sprite_program->require_uniform("tex");
            glUniform1i(tex_uni, 0);  // Texture unit 0
            if (diagnose_opengl("after setting uniform")) {
                throw std::logic_error("image_drawing init failed due to GL error");
            }
        }
    } sprite_layer;

}

using namespace vis;

