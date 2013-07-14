#ifndef HAVE_VIS_SPRITES_H
#define HAVE_VIS_SPRITES_H

#include "common.h"
#include "images.h"
#include "../../util/inc/organization.h"

namespace vis {

    struct Sprites_Renderer {
        struct Data;
        Data* data;
        void draw_sprite (Frame*, Texture*, Vec p, bool fliph = false, bool flipv = false, float z = 0);
    };

    struct Sprites : Linkable<Sprites> {
        virtual void Sprites_draw (Sprites_Renderer) = 0;
        bool visible () { return is_linked(); }
        void appear ();
        void disappear ();
    };

}

#endif
