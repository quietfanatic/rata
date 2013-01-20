#ifndef HAVE_VIS_SPRITES_H
#define HAVE_VIS_SPRITES_H

#include "images.h"
#include "../../util/inc/organization.h"

#define PX (1/16.0)

namespace vis {

    struct Draws_Sprites : Linkable<Draws_Sprites> {
        virtual void draw () = 0;
        bool is_visible () { return is_linked(); }
        void appear ();
        void disappear ();
    };

    void draw_sprite (Frame*, Texture*, Vec, bool, bool, float);

    struct Sprite : Draws_Sprites {
        virtual Texture* sprite_texture () = 0;
        virtual Frame* sprite_frame () = 0;
        virtual Vec sprite_pos () = 0;
        virtual bool sprite_fliph () { return false; }
        virtual bool sprite_flipv () { return false; }
        virtual float sprite_z () { return 0; }
        void draw () {
            draw_sprite(sprite_frame(), sprite_texture(), sprite_pos(), sprite_fliph(), sprite_flipv(), sprite_z());
        }
    };
}

#endif
