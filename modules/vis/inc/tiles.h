#ifndef HAVE_VIS_TILES_H
#define HAVE_VIS_TILES_H

#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"
#include "images.h"

namespace vis {
    using namespace util;

    struct Tiles {
        uint vbo_id = 0;
        uint vao_id = 0;
        size_t vao_size = 0;
         // Call this in your own finish
        void finish (uint32 width, uint32 height, const uint16* tiles);
    };

    void tiles_init ();
    void draw_tiles (Tiles*, Texture*, Vec pos);

}

#endif
