#ifndef HAVE_GEO_TILES_H
#define HAVE_GEO_TILES_H

#include "../inc/rooms.h"
#include "../../phys/inc/phys.h"
#include "../../core/inc/images.h"
#include "../../vis/inc/tiles.h"

namespace geo {

    struct TileDef {
        phys::FixtureDef nature;  // No shape
        std::vector<Vec> vertices;  // The shape is made by merging vertices
    };

    struct Tileset {
        std::vector<TileDef*> tiles;  // TileDefs can be provided with local()
    };

    struct Tilemap : Resident, phys::Object, vis::Tiles {
        Tileset* tileset;
        core::Texture* texture;
        uint32 width;
        uint32 height;
        std::vector<uint16> tiles;

        void Resident_emerge () override;
        void Resident_reclude () override;

        Vec Tiles_pos () override { return Object::pos(); }
        core::Texture* Tiles_texture () override { return texture; }

        Tilemap ();
        void finish ();
    };

    extern Links<Tilemap> active_tilemaps;

}

#endif
