#ifndef HAVE_GEO_CAMERA_H
#define HAVE_GEO_CAMERA_H

#include "../../util/inc/geometry.h"

 // TODO: port the constraint-satisfaction camera control from the old branch.

namespace geo {
    using namespace util;

    extern Vec camera_pos;
    void set_camera_pos (Vec);
    Vec& update_camera ();
     // Make the camera jump straight to its ideal position next frame.
    extern bool camera_jump;
     // While this is true, the camera stays still unless the window cursor
     //  moves
    extern bool free_camera;

//    struct Conspicuous : Linkable<Conspicuous> {
//         // The camera will try to show at least some of this rectangle.
//        virtual Rect conspicuous_area () = 0;
//         // Basically, a priority.
//        virtual float conspicuousity () { return 0; }
//        virtual bool as_close_as_possible () { return false; }
//    };

}

#endif
