#ifndef HAVE_VIS_TILES_H
#define HAVE_VIS_TILES_H

#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"
#include "images.h"

namespace vis {
    using namespace util;

    struct Tiles : Link<Tiles> {
        uint vbo_id;
        uint vao_id;
        size_t vao_size;
        virtual Vec Tiles_pos () = 0;
        virtual vis::Texture* Tiles_texture () = 0;
        void appear ();
        void disappear ();
        bool visible () { return linked(); }
         // Call this in your own finish
        void finish (uint32 width, uint32 height, const uint16* tiles);
    };

     // Not really doing anythig but whatever
    struct Tiles_Renderer {
        struct Data; Data* data;
        Tiles_Renderer ();
        void run ();
    };

}

#endif
