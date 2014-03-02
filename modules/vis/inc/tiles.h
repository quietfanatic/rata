#ifndef HAVE_VIS_TILES_H
#define HAVE_VIS_TILES_H

#include <vector>
#include "util/inc/geometry.h"
#include "util/inc/organization.h"
#include "vis/inc/images.h"

namespace vis {

    struct Tiles {
        uint width = 0;
        uint height = 0;
        std::vector<uint16> tiles;
        uint vbo_id = 0;
        size_t vbo_size = 0;

        void resize (uint, uint);

        void finish ();
        ~Tiles ();
        Tiles () = default;
        Tiles (const Tiles&) = delete;
        Tiles& operator = (const Tiles&) = delete;
        Tiles (Tiles&& o) :
            width(o.width),
            height(o.height),
            tiles(o.tiles),
            vbo_id(o.vbo_id),
            vbo_size(o.vbo_size)
        {
            o.vbo_id = 0;
        }
        Tiles& operator = (Tiles&&) = delete;
    };

    void tiles_init ();
    void draw_tiles (Tiles*, Texture*, util::Vec pos);
    void draw_tile (uint16, Texture*, util::Vec pos);

}

#endif
