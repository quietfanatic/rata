#include "../inc/text.h"
#include "../inc/common.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/opengl.h"

namespace vis {

    using namespace core;

    struct Text_Vert {
        uint16 px;
        uint16 py;
        uint16 tx;
        uint16 ty;
        Text_Vert() { }
        Text_Vert(uint16 px, uint16 py, uint16 tx, uint16 ty) : px(px), py(py), tx(tx), ty(ty) { }
    };

    struct Text_Program : Cameraed_Program {
        GLint model_pos = 0;
        GLint color = 0;
        GLint tex = 0;
        void finish () {
            Cameraed_Program::finish();
            model_pos = require_uniform("model_pos");
            color = require_uniform("color");
            tex = require_uniform("tex");
            glUniform1i(tex, 0);
        }
        void Program_begin () override {
            Cameraed_Program::Program_begin();
             // No VAOs
            glBindVertexArray(0);
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
        }
    };
    static Text_Program* prog = NULL;
    void text_init () {
        prog = hacc::File("vis/res/text.prog").data().attr("prog");
        hacc::manage(&prog);
    }

     // TODO: Make align.x affect each line individually
    Vec draw_text (std::string text, Font* font, Vec pos, Vec align, uint32 color, float wrap) {
        prog->use();
         // Coordinates here work with y being down instead of up.
         //  This may be a little confusing.
        auto verts = new Text_Vert [text.length()][4];
        uint vert_i = 0;
        uint16 maxx = 0;
        uint16 maxy = 0;
        uint16 curx = 0;
        uint16 cury = 0;
        for (uint i = 0; i < text.length(); i++) {
            if (text[i] == '\n') {
                curx = 0;
                cury += font->line_height;
            }
            else {
                uint16 nextx = curx + (font->widths.empty() ? font->width : font->widths[text[i]]);
                if (wrap && nextx > wrap/PX) {
                    curx = 0;
                    nextx = curx + (font->widths.empty() ? font->width : font->widths[text[i]]);
                    cury += font->line_height;
                }
                uint16 tx = text[i] % 16;
                uint16 ty = text[i] / 16;
                verts[vert_i][0] = Text_Vert(curx, cury, tx, ty);
                verts[vert_i][1] = Text_Vert(curx, cury+font->height, tx, ty+1);
                verts[vert_i][2] = Text_Vert(curx+font->width, cury+font->height, tx+1, ty+1);
                verts[vert_i][3] = Text_Vert(curx+font->width, cury, tx+1, ty);
                curx = nextx;
                if (curx > maxx) maxx = curx;
                if (cury + font->line_height > maxy) maxy = cury + font->line_height;
                //printf("'%c' [[[%hu %hu] [%hu %hu]] [[%hu %hu] [%hu %hu]] [[%hu %hu] [%hu %hu]] [[%hu %hu] [%hu %hu]]]\n",
                //    text[i],
                //    verts[vert_i][0].px, verts[vert_i][0].py, verts[vert_i][0].tx, verts[vert_i][0].ty,
                //    verts[vert_i][1].px, verts[vert_i][1].py, verts[vert_i][1].tx, verts[vert_i][1].ty,
                //    verts[vert_i][2].px, verts[vert_i][2].py, verts[vert_i][2].tx, verts[vert_i][2].ty,
                //    verts[vert_i][3].px, verts[vert_i][3].py, verts[vert_i][3].tx, verts[vert_i][3].ty
                //);
                vert_i++;
            }
        }
        Vec size = Vec(maxx*PX, maxy*PX);
        Vec ul = Vec(pos.x - (1 - align.x) / 2 * size.x, pos.y + (1 - align.y) / 2 * size.y);
        glUniform2f(prog->model_pos, ul.x, ul.y);
        glUniform4f(prog->color,
            ((color >> 24) & 255) / 255.0,
            ((color >> 16) & 255) / 255.0,
            ((color >> 8) & 255) / 255.0,
            (color & 255) / 255.0
        );
        glBindTexture(GL_TEXTURE_2D, font->texture->id);
        glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Text_Vert), verts);
        glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Text_Vert), ((char*)verts) + 2*sizeof(uint16));
        glDrawArrays(GL_QUADS, 0, vert_i * 4);
        diagnose_opengl("after drawing some text");
        delete[] verts;
        return size;
    }

    Vec text_size (std::string text, Font* font, float wrap) {
        uint16 maxx = 0;
        uint16 maxy = 0;
        uint16 curx = 0;
        uint16 cury = 0;
        for (uint i = 0; i < text.length(); i++) {
            if (text[i] == '\n') {
                curx = 0;
                cury += font->line_height;
            }
            else {
                uint16 nextx = curx + (font->widths.empty() ? font->width : font->widths[text[i]]);
                if (wrap && nextx > wrap/PX) {
                    curx = 0;
                    nextx = curx + (font->widths.empty() ? font->width : font->widths[text[i]]);
                    cury += font->line_height;
                }
                curx = nextx;
                if (curx > maxx) maxx = curx;
                if (cury + font->line_height > maxy) maxy = cury + font->line_height;
            }
        }
        return Vec(maxx*PX, maxy*PX);
    }

    Vec get_glyph_pos (std::string text, Font* font, uint index, Vec align, float wrap) {
        uint curx = 0;
        uint cury = 0;
        for (uint i = 0; i < index && i < text.length(); i++) {
            if (text[i] == '\n') {
                curx = 0;
                cury += font->line_height;
            }
            else {
                uint16 nextx = curx + (font->widths.empty() ? font->width : font->widths[text[i]]);
                if (wrap && nextx > wrap/PX) {
                    curx = 0;
                    nextx = curx + (font->widths.empty() ? font->width : font->widths[text[i]]);
                    cury += font->line_height;
                }
                curx = nextx;
            }
        }
        return Vec(curx*PX, cury*PX);
    }

} using namespace vis;

HACCABLE(Font) {
    name("vis::Font");
    attr("texture", member(&Font::texture));
    attr("width", member(&Font::width));
    attr("height", member(&Font::height));
    attr("line_height", member(&Font::line_height));
    attr("widths", member(&Font::widths).optional());
}
HCB_INSTANCE(Font*)

HACCABLE(Text_Program) {
    name("vis::Text_Program");
    delegate(base<core::Program>());
    finish(&Text_Program::finish);
}
