#ifndef HAVE_GEO_CAMERA_H
#define HAVE_GEO_CAMERA_H

#include "../../util/inc/geometry.h"

 // TODO: port the constraint-satisfaction camera control from the old branch.

namespace geo {
    using namespace util;

    struct Camera {
        Vec pos = Vec(10, 7.5);
        Vec size = Vec(20, 15);
        Vec& update ();
         // Make the camera jump straight to its ideal position next frame.
        bool jump = false;
         // While this is true, the camera stays still unless the
         //  window cursor moves near the edge.
        bool free = false;
         // TODO: this isn't the best.
        void set_pos (Vec);
         // Chained like core::Listener
        Camera* prev = NULL;
        void activate ();
        void deactivate ();
        void finish () { activate(); }
        ~Camera ();
    };
    extern Camera* camera;

}

#endif
