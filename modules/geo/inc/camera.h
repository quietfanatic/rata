#ifndef HAVE_GEO_CAMERA_H
#define HAVE_GEO_CAMERA_H

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"

namespace geo {

    Vec get_camera_pos ();
     // Make the camera jump straight to its ideal position.
    extern bool camera_jump;

//    struct Conspicuous : Linkable<Conspicuous> {
//         // The camera will try to show at least some of this rectangle.
//        virtual Rect conspicuous_area () = 0;
//         // Basically, a priority.
//        virtual float conspicuousity () { return 0; }
//        virtual bool as_close_as_possible () { return false; }
//    };

}

#endif
