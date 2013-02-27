#ifndef HAVE_CORE_INPUT_H
#define HAVE_CORE_INPUT_H

#include <GL/glfw.h>
#include "../../util/inc/honestly.h"

namespace core {
    uint8 get_key (uint code);
    void temp_key_cb (void GLFWCALL (* cb ) (int keycode, int action));
    void undo_temp_key_cb ();
}

#endif
