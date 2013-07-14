#ifndef HAVE_VIS_SPRITES_H
#define HAVE_VIS_SPRITES_H

#include "common.h"
#include "images.h"
#include "../../util/inc/organization.h"

namespace vis {

    struct Sprites_Renderer {
        struct Data; Data* data;
        Sprites_Renderer ();
        void run ();
        void draw_sprite (Frame*, Texture*, Vec p, bool fliph = false, bool flipv = false, float z = 0);
    };

    struct Sprites : Link<Sprites> {
        virtual void Sprites_draw (Sprites_Renderer) = 0;
        void appear ();
        void disappear ();
        bool visible () { return linked(); }
    };

}

#endif
