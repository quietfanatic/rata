#include "../inc/color.h"
#include "../inc/common.h"
#include "../../hacc/inc/everything.h"

namespace vis {

    struct Color_Program : Cameraed_Program {
        GLint model_pos;
        GLint color;
        void finish () {
            Cameraed_Program::finish();
            model_pos = require_uniform("model_pos");
            color = require_uniform("color");
        }
    };

    static Color_Program* prog = NULL;
    void color_init () {
        prog = hacc::File("vis/res/color.prog").data().attr("prog");
        hacc::manage(&prog);
    }
    void color_offset (Vec off) {
        prog->use();
        glUniform2f(prog->model_pos, round(off.x/PX)*PX, round(off.y/PX)*PX);
    }
    void draw_color (uint32 color) {
        prog->use();
        glUniform4f(prog->color,
            ((color >> 24) & 255) / 255.0,
            ((color >> 16) & 255) / 255.0,
            ((color >> 8) & 255) / 255.0,
            (color & 255) / 255.0
        );
    }
    void line_width (uint width) {
        prog->use();
        glLineWidth(width);
    }
    void draw_line (Vec a, Vec b) {
        Vec pts [2];
        pts[0] = a;
        pts[1] = b;
        draw_chain(2, pts);
    }
    void draw_rect (const Rect& r) {
        Vec pts [4];
        pts[0] = r.lb();
        pts[1] = r.rb();
        pts[2] = r.rt();
        pts[3] = r.lt();
        draw_loop(4, pts);
    }
    void draw_solid_rect (const Rect& r) {
        Vec pts [4];
        pts[0] = r.lb();
        pts[1] = r.rb();
        pts[2] = r.rt();
        pts[3] = r.lt();
        draw_primitive(GL_QUADS, 4, pts);
    }
    void draw_chain (size_t n, Vec* pts) {
        draw_primitive(GL_LINE_STRIP, n, pts);
    }
    void draw_loop (size_t n, Vec* pts) {
        draw_primitive(GL_LINE_LOOP, n, pts);
    }
    void draw_primitive (uint type, size_t n, Vec* pts) {
        prog->use();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, pts);
        glDrawArrays(type, 0, n);
        glDisableVertexAttribArray(0);
        core::diagnose_opengl("after Color_Program::primitive");
    }

} using namespace vis;

HACCABLE(Color_Program) {
    name("vis::Color_Program");
    delegate(base<core::Program>());
    finish(&Color_Program::finish);
}
