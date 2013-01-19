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

    void draw_sprite (Image*, SubImg*, Vec, bool, bool);

    struct Sprite : Draws_Sprites {
        virtual Image* img_image () = 0;
        virtual SubImg* img_sub () = 0;
        virtual Vec img_pos () = 0;
        virtual bool img_fliph () { return false; }
        virtual bool img_flipv () { return false; }
        void draw () {
            draw_sprite(img_image(), img_sub(), img_pos(), img_fliph(), img_flipv());
        }
    };

}

#endif
