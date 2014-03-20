#ifndef HAVE_VIS_COLOR_H
#define HAVE_VIS_COLOR_H

#include "util/inc/geometry.h"

 // This draws lines and shapes of alpha-blended color.

namespace vis {
    void color_init ();

    void color_offset (util::Vec);
    void draw_color (uint32);
    void line_width (uint width = 1);
    void draw_line (util::Vec, util::Vec);
    void draw_rect (const util::Rect&);
    void draw_solid_rect (const util::Rect&);
    void draw_circle (const util::Circle&);
    void draw_chain (size_t, util::Vec*);
    void draw_loop (size_t, util::Vec*);
    void draw_primitive (uint type, size_t, util::Vec*);

}

#endif
