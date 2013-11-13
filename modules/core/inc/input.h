#ifndef HAVE_CORE_INPUT_H
#define HAVE_CORE_INPUT_H

#include <GL/glfw.h>
#include "../../util/inc/honestly.h"
#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"

 // TODO: make up our mind about whether our input system should be
 //  event-based or poll-based.

namespace core {
    using namespace util;

    bool key_pressed (int keycode);
    bool btn_pressed (int code);
     // This is only updated when the cursor is not trapped.
    extern Vec window_cursor_pos;
     // Motion this frame.  This is 0,0 if the cursor is not trapped.
    extern Vec cursor_motion;

    void run_input ();
}

#endif
