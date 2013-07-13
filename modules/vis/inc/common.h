#ifndef HAVE_VIS_COMMON_H
#define HAVE_VIS_COMMON_H

#include "../../util/inc/geometry.h"

 // The vis namespace handles all rendering (though not all opengl
 //  integration; some of that's in core).

namespace vis {
    using namespace util;

     // Setting this alters all rendering.
    extern Vec camera_pos;

}

#endif
