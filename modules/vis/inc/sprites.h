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

    void draw_sprite (Frame*, core::Texture*, Vec p, bool fliph = false, bool flipv = false, float z = 0);

    struct Sprite : Linkable<Sprite> {
        virtual void Sprite_draw () = 0;
        bool visible () { return is_linked(); }
        void appear ();
        void disappear ();
    };

     // We're stuffing rendering camera control here for lack of a better place
    extern Vec camera_pos;

}

#endif
