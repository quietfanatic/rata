
#include <GL/gl.h>
#include "../../core/inc/game.h"
#include "../inc/graffiti.h"
#include "../inc/shaders.h"
#include "../../hacc/inc/haccable_files.h"

namespace vis {

    static vis::Program* graffiti_program;

    struct Graffiti_Layer : core::Layer {
        Graffiti_Layer () : core::Layer("F.M", "graffiti") { }
        void init () {
            graffiti_program = hacc::reference_file<Program>("modules/vis/res/color.prog");
        }
        void run () {
            glLoadIdentity();  // MODELVIEW matrix
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
        graffiti_program->use();
        glColor4ub(color >> 24, color >> 16, color >> 8, color);
        glBegin(type);
        for (uint i = 0; i < n_pts; i++)
            glVertex2f(pts[i].x, pts[i].y);
        glEnd();
    }
    void shift_graffiti (Vec new_pos) {
        glLoadIdentity();
        glTranslatef(new_pos.x, new_pos.y, 0);
    }

}

