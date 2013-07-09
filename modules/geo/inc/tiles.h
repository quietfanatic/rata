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

    struct Tilemap : Resident, phys::Object, Linkable<Tilemap> {
         // Using Object::pos for position
        Tileset* tileset;
        core::Texture* texture;
        uint32 width;
        uint32 height;
        std::vector<uint16> tiles;
        uint vbo_id;
        uint vao_id;
        size_t vao_size;

        void Resident_emerge () override;
        void Resident_reclude () override;

        Tilemap ();
        void finish ();
    };

    extern Links<Tilemap> active_tilemaps;

}

#endif
