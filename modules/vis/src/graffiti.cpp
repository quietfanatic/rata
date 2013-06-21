
#include <GL/gl.h>
#include "../inc/graffiti.h"
#include "../../core/inc/opengl.h"
#include "../../core/inc/phases.h"
#include "../../core/inc/state.h"
#include "../../hacc/inc/haccable_files.h"

namespace vis {

    using namespace core;

    struct Graffiti_Renderer : Renderer, Game_Object {
        Program* program = hacc::reference_file<Program>("modules/vis/res/color.prog");
        int camera_pos = program->require_uniform("camera_pos");
        int model_pos = program->require_uniform("model_pos");
        int color = program->require_uniform("color");
        void start () { }
        void start_rendering () {
            static auto glUniform2f = glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
            static auto glBindVertexArray = glproc<void (GLuint)>("glBindVertexArray");
            static auto glEnableVertexAttribArray = glproc<void (GLuint)>("glEnableVertexAttribArray");
            static auto glDisableVertexAttribArray = glproc<void (GLuint)>("glDisableVertexAttribArray");
            static auto glUseProgram = glproc<void (GLuint)>("glUseProgram");
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBindVertexArray(0);
            glEnableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glUseProgram(program->glid);
            glUniform2f(camera_pos, 10.0, 7.5);
        }
    };
    core::Celebrity<Graffiti_Renderer> gr;

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
        gr->use();
        glUniform4f(gr->color,
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
        gr->use();
        glUniform2f(gr->model_pos, pos.x, pos.y);
    }

}

