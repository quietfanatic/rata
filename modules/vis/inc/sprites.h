#ifndef HAVE_VIS_SPRITES_H
#define HAVE_VIS_SPRITES_H

#include "common.h"
#include "../../core/inc/images.h"
#include "../../util/inc/organization.h"

namespace vis {

    struct Sprite : Linkable<Sprite> {
        virtual void Sprite_draw () = 0;
        bool visible () { return is_linked(); }
        void appear ();
        void disappear ();
    };

     // Call this in Sprite_draw.  You can call it elsewhere I guess, but if
     //  it's not called during the Sprite layer, results may be undefined.
    void draw_sprite (Frame*, core::Texture*, Vec p, bool fliph = false, bool flipv = false, float z = 0);

}

#endif
