#ifndef HAVE_GEO_TILES_H
#define HAVE_GEO_TILES_H

#include <vector>
#include "geo/inc/rooms.h"
#include "phys/inc/phys.h"
#include "vis/inc/common.h"
#include "vis/inc/tiles.h"

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

    struct Tilemap_Def : phys::Object_Def {
        Tileset* tileset = NULL;
        vis::Tiles* tiles = NULL;
        vis::Texture* texture = NULL;
    };

     // A visible, tangible object that uses a matrix of tiles to create
     //  level geometry.
    struct Tilemap : Resident, phys::Object, vis::Drawn<vis::Map> {
        Tilemap_Def* get_def () { return static_cast<Tilemap_Def*>(def); }

        void Resident_emerge () override;
        void Resident_reclude () override;

        void Drawn_draw (vis::Map) override;

        void physicalize ();
        void finish ();
    };

}

#endif
