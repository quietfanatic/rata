#ifndef HAVE_VIS_COMMON_H
#define HAVE_VIS_COMMON_H

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"

 // The vis namespace handles all rendering (though not all opengl
 //  integration; some of that's in core).

namespace vis {

     // Setting this alters all rendering.
    extern Vec camera_pos;

    struct Layout;
    struct Frame;

    struct Layout {
        Vec size;
        std::vector<Frame> frames;
        uint vbo_id;
        uint vao_id;
        void finish ();
        ~Layout ();
         // Hacc attribute lookup goes here
        Frame* frame_named (std::string);
    };

    struct Frame {
        Layout* parent;
        std::string name;
        Vec offset;
        Rect box;
        std::vector<Vec> points;
    };
}

#endif
