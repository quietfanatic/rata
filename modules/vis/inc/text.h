#ifndef HAVE_VIS_TEXT_H
#define HAVE_VIS_TEXT_H

#include <string>
#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"
#include "images.h"

namespace vis {
    using namespace util;

     // A font's image should be composed of 16x16 tiles.
    struct Font {
        vis::Texture* texture;
        uint width;
        uint height;
        uint line_height;
        std::vector<uint8> widths;
    };

    extern Font* console_font;

    struct Text_Renderer {
        struct Data;
        Data* data;
         // Draws a string of text with a transparent background.
         // Multiple lines will be separated by font->line_height.
         // align.x: 1 -> left justification, 0 -> center, -1 -> right
         // align.y: 1 -> top of text is at pos, 0 -> center, -1 -> bottom
         //  horizontal alignment affects each line individually.
         // Returns the size of the rectangle encompassing the drawn text.
        Vec draw_text (std::string text, Font* font, Vec pos, Vec align = Vec(1, 1), uint32 color = 0xffffffff, float wrap = 0);
    };

    struct Text : Linkable<Text> {
        virtual void Text_draw (Text_Renderer) = 0;
        bool visible () { return is_linked(); }
        void appear ();
        void disappear ();
    };

     // Returns the size of the rectangle encompassing the drawn text.
    Vec text_size (std::string text, Font* font, float wrap = 0);
     // Gets the position of the upper-left corner of the ith character in text,
     //  relative to the upper-left corner of the whole text.
    Vec get_glyph_pos (std::string text, Font* font, uint index, Vec align = Vec(1, 1), float wrap = 0);

}

#endif
