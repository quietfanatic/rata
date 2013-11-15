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

    void text_init ();

    extern Font* default_font;

     // Returns the size of the rectangle encompassing the drawn text.
    Vec draw_text (std::string text, Font* font, Vec pos, Vec align = Vec(1, 1), uint32 color = 0xffffffff, float wrap = 0);
     // Returns size without drawing
    Vec text_size (std::string text, Font* font, float wrap = 0);
     // Gets the position of the upper-left corner of the ith character in text,
     //  relative to the upper-left corner of the whole text.
    Vec get_glyph_pos (std::string text, Font* font, uint index, Vec align = Vec(1, 1), float wrap = 0);

}

#endif
