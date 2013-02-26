#ifndef HAVE_VIS_TEXT_H
#define HAVE_VIS_TEXT_H

#include <string>
#include "../../util/inc/Vec.h"
#include "images.h"

namespace vis {

     // A font's image should be composed of 16x16 tiles.
    struct Font {
        Texture* texture;
        uint width;
        uint height;
        uint line_height;
        std::vector<uint8> widths;
    };

     // Draws a string of text with a transparent background.
     // Multiple lines will be separated by font->line_height.
     // align.x: 1 -> left justification, 0 -> center, -1 -> right
     // align.y: 1 -> top of text is at pos, 0 -> center, -1 -> bottom
     //  horizontal alignment affects each line individually.
    void draw_text (std::string text, Font* font, Vec pos, Vec align = Vec(1, 1), uint32 color = 0xffffffff);

}

#endif
