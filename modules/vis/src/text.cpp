#include "vis/inc/text.h"

#include "core/inc/commands.h"
#include "core/inc/opengl.h"
#include "hacc/inc/everything.h"
#include "vis/inc/common.h"
using namespace core;
using namespace util;

namespace vis {


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
    };

    static Text_Program* prog = NULL;

    Font* default_font = NULL;

    void text_init () {
        prog = hacc::File("vis/res/text.prog").attr("prog");
        hacc::manage(&prog);
        hacc::manage(&default_font);
    }

    static Font* use_default (Font* font) {
        if (font) return font;
        if (default_font) return default_font;
        throw hacc::X::Logic_Error("No font provided and no default font set");
    }

     // TODO: Make align.x affect each line individually
    Vec draw_text (std::string text, Font* font, Vec pos, Vec align, uint32 color, float wrap) {
        prog->use();
        font = use_default(font);
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
                vert_i++;
            }
        }
        Vec size = Vec(maxx*PX, maxy*PX);
        Vec ul = Vec(pos.x - (1 - align.x) / 2 * size.x, pos.y + (1 - align.y) / 2 * size.y);
        glUniform2f(prog->model_pos, round(ul.x/PX)*PX, round(ul.y/PX)*PX);
        glUniform4f(prog->color,
            ((color >> 24) & 255) / 255.0,
            ((color >> 16) & 255) / 255.0,
            ((color >> 8) & 255) / 255.0,
            (color & 255) / 255.0
        );
        glBindTexture(GL_TEXTURE_2D, font->texture->id);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Text_Vert), verts);
        glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Text_Vert), ((char*)verts) + 2*sizeof(uint16));
        glDrawArrays(GL_QUADS, 0, vert_i * 4);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        diagnose_opengl("after drawing some text");
        delete[] verts;
        return size;
    }

    Vec text_size (std::string text, Font* font, float wrap) {
        font = use_default(font);
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
        font = use_default(font);
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

void _set_default_font (Font* f) { default_font = f; }
New_Command _set_default_font_cmd ("set_default_font", "Set the default font", 1, _set_default_font);
