#ifndef HAVE_VIS_SPRITES_H
#define HAVE_VIS_SPRITES_H

#include "../../util/inc/Rect.h"
#include "../../core/inc/images.h"
#include "../../util/inc/organization.h"

#define PX (1/16.0)

namespace vis {

    struct Layout;

    struct Frame {
        Layout* parent;
        std::string name;
        Vec offset;
        Rect box;
        std::vector<Vec> points;
    };

    struct Layout {
        Vec size;
        std::vector<Frame> frames;
        uint vbo_id;
        uint vao_id;

        void finish ();
        ~Layout ();

        Frame* frame_named (std::string);
    };


    struct Draws_Sprites : Linkable<Draws_Sprites> {
        virtual void draws_sprites () = 0;
        bool is_active () { return is_linked(); }
        void activate ();
        void deactivate ();
        static void draw_sprite (Frame*, core::Texture*, Vec p, bool fliph = false, bool flipv = false, float z = 0);
    };

}

#endif
