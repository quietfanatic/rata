#ifndef HAVE_VIS_TEXT_H
#define HAVE_VIS_TEXT_H

#include <string>
#include "util/inc/geometry.h"
#include "util/inc/organization.h"
#include "vis/inc/images.h"

namespace vis {

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
    util::Vec draw_text (std::string text, Font* font, util::Vec pos, util::Vec align = util::Vec(1, 1), uint32 color = 0xffffffff, float wrap = 0);
     // Returns size without drawing
    util::Vec text_size (std::string text, Font* font, float wrap = 0);
     // Gets the position of the upper-left corner of the ith character in text,
     //  relative to the upper-left corner of the whole text.
    util::Vec get_glyph_pos (std::string text, Font* font, uint index, util::Vec align = util::Vec(1, 1), float wrap = 0);

}

#endif
