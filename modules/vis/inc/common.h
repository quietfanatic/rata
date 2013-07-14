#ifndef HAVE_VIS_COMMON_H
#define HAVE_VIS_COMMON_H

#include "../../util/inc/geometry.h"

 // The vis namespace handles all rendering (though not all opengl
 //  integration; some of that's in core).

namespace vis {
    using namespace util;

     // Setting this alters rendering until the HUD step.
    extern Vec camera_pos;

     // This can be called automatically, but it's better to do it manually,
     //  to avoid first-frame lag.
    void init ();

     // Do it.
    void render ();

}

#endif
