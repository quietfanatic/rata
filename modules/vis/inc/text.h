#ifndef HAVE_VIS_TEXT_H
#define HAVE_VIS_TEXT_H

#include <string>
#include "../../util/inc/Vec.h"
#include "../../util/inc/organization.h"
#include "../../core/inc/images.h"

namespace vis {

     // A font's image should be composed of 16x16 tiles.
    struct Font {
        core::Texture* texture;
        uint width;
        uint height;
        uint line_height;
        std::vector<uint8> widths;
    };

    extern Font* console_font;

     // Draws a string of text with a transparent background.
     // Multiple lines will be separated by font->line_height.
     // align.x: 1 -> left justification, 0 -> center, -1 -> right
     // align.y: 1 -> top of text is at pos, 0 -> center, -1 -> bottom
     //  horizontal alignment affects each line individually.
     // Returns the size of the rectangle encompassing the drawn text.
    Vec draw_text (std::string text, Font* font, Vec pos, Vec align = Vec(1, 1), uint32 color = 0xffffffff, float wrap = 0);

    struct Draws_Text : Linkable<Draws_Text> {
        virtual void text_draw () = 0;
        bool text_is_visible () { return is_linked(); }
        void text_appear ();
        void text_disappear ();
    };
     // Returns the size of the rectangle encompassing the drawn text.
    Vec text_size (std::string text, Font* font, float wrap = 0);
     // Gets the position of the upper-left corner of the ith character in text,
     //  relative to the upper-left corner of the whole text.
    Vec get_glyph_pos (std::string text, Font* font, uint index, Vec align = Vec(1, 1), float wrap = 0);

}

#endif
