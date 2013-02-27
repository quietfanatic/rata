#ifndef HAVE_CORE_INPUT_H
#define HAVE_CORE_INPUT_H

#include <GL/glfw.h>
#include "../../util/inc/honestly.h"
#include "../../util/inc/organization.h"

namespace core {

    struct Key_Listener : Linkable<Key_Listener> {
         // Return true if you've handled the key.
        virtual bool hear_key (int keycode, int action) = 0;
        void activate ();
        void deactivate ();
    };

}

#endif
