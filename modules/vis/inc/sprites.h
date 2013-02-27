#ifndef HAVE_VIS_SPRITES_H
#define HAVE_VIS_SPRITES_H

#include "images.h"
#include "../../util/inc/organization.h"

#define PX (1/16.0)

namespace vis {

    struct Draws_Sprites : Linkable<Draws_Sprites> {
        virtual void draws_sprites () = 0;
        bool is_active () { return is_linked(); }
        void activate ();
        void deactivate ();
        static void draw_sprite (Frame*, Texture*, Vec p, bool fliph = false, bool flipv = false, float z = 0);
    };

}

#endif
