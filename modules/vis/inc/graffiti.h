#ifndef HAVE_VIS_GRAFFITI_H
#define HAVE_VIS_GRAFFITI_H

#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"

namespace vis {
    using namespace util;

    struct Graffiti_Renderer {
        struct Data;
        Data* data;
         // This will be added to coordinates while rendering this graffiti object.
         // It will not affect the next Graffiti object.
        void offset (Vec pos);
        void draw_line (Vec a, Vec b, uint32 color, float width = 1);
        void draw_chain (uint n_pts, Vec* pts, uint32 color, float width = 1);
        void draw_loop (uint n_pts, Vec* pts, uint32 color, float width = 1);
        void draw_primitive (uint type, uint n_pts, Vec* pts, uint32 color);
    };

    struct Graffiti : Linkable<Graffiti> {
        virtual void Graffiti_draw (Graffiti_Renderer) = 0;
        void appear ();
        void disappear ();
        bool visible () { return is_linked(); }
    };

}

#endif
