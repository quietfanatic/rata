#ifndef HAVE_GEO_TILES_H
#define HAVE_GEO_TILES_H

#include "../inc/rooms.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/tiles.h"

namespace geo {

     // Specifies the properties of one type of tile.
    struct TileDef {
        phys::FixtureDef nature;  // No shape
        std::vector<Vec> vertices;  // The shape is made by merging vertices
    };

     // A set of TileDefs with integer indexes.
    struct Tileset {
        std::vector<TileDef*> tiles;  // TileDefs can be provided with local()
    };

     // A visible, tangible object that uses a matrix of tiles to create
     //  level geometry.
    struct Tilemap : Resident, phys::Object, vis::Drawn<vis::Map> {
        Tileset* tileset;
        vis::Texture* texture;
        vis::Tiles* tiles;

        void Resident_emerge () override;
        void Resident_reclude () override;

        void Drawn_draw (vis::Map) override;

        Tilemap ();
        void physicalize ();
        void finish ();
    };

    extern Links<Tilemap> active_tilemaps;

}

#endif
