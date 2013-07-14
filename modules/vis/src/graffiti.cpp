
#include <GL/gl.h>
#include "../inc/graffiti.h"
#include "../inc/sprites.h"
#include "../../hacc/inc/files.h"
#include "../../core/inc/opengl.h"
#include "../../core/inc/phases.h"

namespace vis {
    using namespace core;

    Links<Graffiti> graffiti_drawers;
    void Graffiti::appear () { link(graffiti_drawers); }
    void Graffiti::disappear () { unlink(); }

    struct Graffiti_Renderer::Data {
        Program* program = hacc::File("modules/vis/res/color.prog").data().attr("prog");
        int camera_pos = program->require_uniform("camera_pos");
        int model_pos = program->require_uniform("model_pos");
        int color = program->require_uniform("color");
    };

    Graffiti_Renderer::Graffiti_Renderer () : data(new Data) { }

    void Graffiti_Renderer::run () {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBindVertexArray(0);
        glEnableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glUseProgram(data->program->glid);
        glUniform2f(data->camera_pos, vis::camera_pos.x, vis::camera_pos.y);
        for (auto& g : graffiti_drawers) {
            g.Graffiti_draw(*this);
        }
    }

    void Graffiti_Renderer::draw_line (Vec a, Vec b, uint32 color, float width) {
        Vec pts [2];
        pts[0] = a;
        pts[1] = b;
        draw_chain(2, pts, color, width);
    }
    void Graffiti_Renderer::draw_chain (uint n_pts, Vec* pts, uint32 color, float width) {
        glLineWidth(width);
        draw_primitive(GL_LINE_STRIP, n_pts, pts, color);
    }
    void Graffiti_Renderer::draw_loop (uint n_pts, Vec* pts, uint32 color, float width) {
        glLineWidth(width);
        draw_primitive(GL_LINE_LOOP, n_pts, pts, color);
    }
    void Graffiti_Renderer::draw_primitive (uint type, uint n_pts, Vec* pts, uint32 color) {
        glUniform4f(data->color,
            ((color >> 24) & 255) / 255.0,
            ((color >> 16) & 255) / 255.0,
            ((color >> 8) & 255) / 255.0,
            (color & 255) / 255.0
        );
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, pts);
        glDrawArrays(type, 0, n_pts);
        diagnose_opengl("after drawing some graffiti");
    }
    void Graffiti_Renderer::offset (Vec pos) {
        glUniform2f(data->model_pos, pos.x, pos.y);
    }

}

