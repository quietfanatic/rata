#ifndef HAVE_GEO_TILES_H
#define HAVE_GEO_TILES_H

#include "../inc/rooms.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/images.h"

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
        vis::Texture* texture;
        Vec pos;
        uint32 width;
        uint32 height;
        std::vector<uint16> tiles;

        void emerge ();
        void reclude ();
        void start ();
        Tilemap ();
    };

    extern Links<Tilemap> active_tilemaps;

}

#endif
