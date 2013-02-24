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
     // uniforms
    GLint sprite_program_tex = 0;
    GLint sprite_program_camera_pos = 0;
    GLint sprite_program_model_pos = 0;
    GLint sprite_program_model_scale = 0;
     // vertex attributes
//    GLint sprite_program_vert_pos = 0;
//    GLint sprite_program_vert_tex = 1;

    void draw_sprite (Frame* frame, Texture* tex, Vec p, bool fliph, bool flipv, float z) {
        static auto glBindVertexArray = glproc<void (GLuint)>("glBindVertexArray");
        static auto glUniform2f = glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
        static auto glUniform3f = glproc<void (GLint, GLfloat, GLfloat, GLfloat)>("glUniform3f");

        if (draw_sprite_logger.on) {
            draw_sprite_logger.log("tex: %s frame: [%g %g] [%g %g %g %g] p: [%g %g] fliph: %u flipv: %u, z: %g",
                tex ? tex->name.c_str() : "NULL", frame ? frame->offset.x : 0/0.0, frame ? frame->offset.y : 0/0.0,
                frame ? frame->box.l : 0/0.0, frame ? frame->box.b : 0/0.0, frame ? frame->box.r : 0/0.0, frame ? frame->box.t : 0/0.0,
                p.x, p.y, fliph, flipv, z
            );
        }


        sprite_program->use();
        glUniform3f(sprite_program_model_pos, p.x, p.y, z);
        glUniform2f(sprite_program_model_scale, fliph ? -1.0 : 1.0, flipv ? -1.0 : 1.0);
        glBindTexture(GL_TEXTURE_2D, tex->tex);
        glBindVertexArray(frame->parent->vao_id);
        glDrawArrays(GL_QUADS, 4 * (frame - frame->parent->frames.data()), 4);

        diagnose_opengl("After rendering a sprite");
    }

    struct Camera_Setup_Layer : core::Layer {
        Camera_Setup_Layer () : core::Layer("B.M", "camera_setup") { }
        void run () {
            static auto glUniform2f = glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
            glClearColor(0.5, 0.5, 0.5, 0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            glDisable(GL_BLEND);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST); // Depth buffer is awesome
            sprite_program->use();
            glUniform2f(sprite_program_camera_pos, 10, 7.5);  // TODO: Control the camera with this
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
            sprite_program = hacc::reference_file<Program>("modules/vis/res/sprite.prog");
            sprite_program->use();
            sprite_program_tex = sprite_program->require_uniform("tex");
            sprite_program_camera_pos = sprite_program->require_uniform("camera_pos");
            sprite_program_model_pos = sprite_program->require_uniform("model_pos");
            sprite_program_model_scale = sprite_program->require_uniform("model_scale");
            glUniform1i(sprite_program_tex, 0);  // Texture unit 0
            if (diagnose_opengl("after setting uniforms and stuff")) {
                throw std::logic_error("sprites init failed due to GL error");
            }
        }
    } sprite_layer;

}

using namespace vis;

