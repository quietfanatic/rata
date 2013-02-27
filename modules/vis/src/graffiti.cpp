
#include <GL/gl.h>
#include "../../core/inc/game.h"
#include "../inc/graffiti.h"
#include "../inc/shaders.h"
#include "../../hacc/inc/haccable_files.h"

namespace vis {

    static vis::Program* graffiti_program;
    static int graffiti_program_camera_pos = 0;
    static int graffiti_program_model_pos = 0;
    static int graffiti_program_color = 0;

    struct Graffiti_Layer : core::Layer {
        Graffiti_Layer () : core::Layer("F.M", "graffiti") { }
        void init () {
            graffiti_program = hacc::reference_file<Program>("modules/vis/res/color.prog");
            graffiti_program_camera_pos = graffiti_program->require_uniform("camera_pos");
            graffiti_program_model_pos = graffiti_program->require_uniform("model_pos");
            graffiti_program_color = graffiti_program->require_uniform("color");
        }
        void run () {
            static auto glBindVertexArray = glproc<void (GLuint)>("glBindVertexArray");
            static auto glEnableVertexAttribArray = glproc<void (GLuint)>("glEnableVertexAttribArray");
            static auto glUniform2f = glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBindVertexArray(0);
            glEnableVertexAttribArray(0);
            graffiti_program->use();
            glUniform2f(graffiti_program_camera_pos, 10.0, 7.5);
            glUniform2f(graffiti_program_model_pos, 0, 0);
        }
    } graffiti_layer;

    void draw_line (Vec a, Vec b, uint32 color, float width) {
        Vec pts [2];
        pts[0] = a;
        pts[1] = b;
        draw_chain(2, pts, color, width);
    }
    void draw_chain (uint n_pts, Vec* pts, uint32 color, float width) {
        glLineWidth(width);
        draw_primitive(GL_LINE_STRIP, n_pts, pts, color);
    }
    void draw_loop (uint n_pts, Vec* pts, uint32 color, float width) {
        glLineWidth(width);
        draw_primitive(GL_LINE_LOOP, n_pts, pts, color);
    }
    void draw_primitive (uint type, uint n_pts, Vec* pts, uint32 color) {
        static auto glUniform4f = glproc<void (GLint, GLfloat, GLfloat, GLfloat, GLfloat)>("glUniform4f");
        static auto glVertexAttribPointer = glproc<void (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*)>("glVertexAttribPointer");
        graffiti_program->use();
        glUniform4f(graffiti_program_color,
            ((color >> 24) & 255) / 255.0,
            ((color >> 16) & 255) / 255.0,
            ((color >> 8) & 255) / 255.0,
            (color & 255) / 255.0
        );
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, pts);
        glDrawArrays(type, 0, n_pts);
        diagnose_opengl("after drawing some graffiti");
    }
    void graffiti_pos (Vec pos) {
        static auto glUniform2f = glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
        glUniform2f(graffiti_program_model_pos, pos.x, pos.y);
    }

}

