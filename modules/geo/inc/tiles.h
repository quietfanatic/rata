#ifndef HAVE_GEO_TILES_H
#define HAVE_GEO_TILES_H

#include "../inc/rooms.h"
#include "../../phys/inc/phys.h"
#include "../../core/inc/images.h"

namespace geo {

    struct TileDef {
        phys::FixtureDef nature;  // No shape
        std::vector<Vec> vertices;  // The shape is made by merging vertices
    };

    struct Tileset {
        std::vector<TileDef*> tiles;  // TileDefs can be provided with local()
    };

    struct Tilemap : Furniture, phys::Object, Linkable<Tilemap> {
        Tileset* tileset;
        core::Texture* texture;
        Vec pos;
        uint32 width;
        uint32 height;
        std::vector<uint16> tiles;
        uint vbo_id;
        uint vao_id;
        size_t vao_size;

        void emerge ();
        void reclude ();
        void start ();

        Tilemap ();
    };

    extern Links<Tilemap> active_tilemaps;

}

#endif
