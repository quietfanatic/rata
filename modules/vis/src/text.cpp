#include "../inc/text.h"
#include "../../hacc/inc/haccable_standard.h"

namespace vis {

    struct Text_Vert {
        uint16 px;
        uint16 py;
        uint16 tx;
        uint16 ty;
        Text_Vert(uint16 px, uint16 py, uint16 tx, uint16 ty) : px(px), py(py), tx(tx), ty(ty) { }
    };

     // TODO: Make align.x affect each line individually
    void draw_text (std::string text, Font font, Vec pos, Vec align, uint32 color) {
        Text_Vert* verts = new Text_Vert [text.length()][4];
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
                uint16 tx = text[i] % 16;
                uint16 ty = text[i] / 16;
                verts[vert_i][0] = Text_Vert(curx, cury, tx, ty);
                verts[vert_i][1] = Text_Vert(curx, cury+font->height, tx, ty+1);
                verts[vert_i][2] = Text_Vert(curx+font->width, cury+font->height, tx+1, ty+1);
                verts[vert_i][3] = Text_Vert(curx+font->width, cury, tx+1, ty);
                curx += font->widths[text[i]];
                if (curx > maxx) maxx = curx;
                if (cury > maxy) maxy = cury;
            }
        }
        static auto glUniform2f = glproc<void (GLint, GLfloat, GLfloat)>("glUniform2f");
        static auto glUniform4f = glproc<void (GLint, GLfloat, GLfloat, GLfloat, GLfloat)>("glUniform4f");
        static auto glBindVertexArray = glproc<void (GLuint)>("glBindVertexArray");
        static auto glVertexAttribPointer = glproc<void (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*)>("glVertexAttribPointer");
        Vec size = Vec(maxx*PX, maxy*PX);
        Vec ul = pos + (Vec(1, 1) - align).scale(size) / 2;
        glBindVertexArray(0);
        text_program->use();
        glUniform2f(text_program_model_pos, ul.x, ul.y);
        glUniform4f(text_program_color,
            ((color >> 24) & 255) / 255.0,
            ((color >> 16) & 255) / 255.0,
            ((color >> 8) & 255) / 255.0,
            (color & 255) / 255.0
        );
        glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Text_Vert), &verts);
        glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Text_Vert), ((void*)&verts) + 2*sizeof(uint16));
        glDrawArrays(GL_QUADS, 0, vert_i);
        diagnose_opengl("after drawing some text");
        delete[] verts;
    }

}

using namespace vis;
HCB_BEGIN(Font)
    type_name("vis::Font");
    attr("texture", member(&Font::texture));
    attr("width", member(&Font::width));
    attr("height", member(&Font::height));
    attr("line_height", member(&Font::line_height));
    attr("width_lists", member(&Font::width_lists));
HCB_END(Font)

