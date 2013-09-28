#ifndef HAVE_VIS_COLOR_H
#define HAVE_VIS_COLOR_H

#include "../../util/inc/geometry.h"

 // This draws lines and shapes of alpha-blended color.

namespace vis {
    using namespace util;

    void color_init ();

    void color_offset (Vec);
    void draw_color (uint32);
    void line_width (uint width = 1);
    void draw_line (Vec, Vec);
    void draw_chain (size_t, Vec*);
    void draw_loop (size_t, Vec*);
    void draw_primitive (uint type, size_t, Vec*);

}

#endif
