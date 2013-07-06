#ifndef HAVE_CORE_INPUT_H
#define HAVE_CORE_INPUT_H

#include <GL/glfw.h>
#include "../../util/inc/honestly.h"
#include "../../util/inc/organization.h"

namespace core {

    struct Key_Listener;
    extern std::vector<Key_Listener*> key_listeners;
    struct Key_Listener : Ordered<Key_Listener, key_listeners> {
         // Return true if you've handled the key.
        virtual bool hear_key (int keycode, int action) = 0;

        Key_Listener (std::string order = "Z") : Ordered(order) { }

        void activate ();
        void deactivate ();
    };

    struct Char_Listener;
    extern std::vector<Char_Listener*> char_listeners;
    struct Char_Listener : Ordered<Char_Listener, char_listeners> {
         // Return true if you've handled the key.
        virtual bool hear_char (int code, int action) = 0;

        Char_Listener (std::string order = "Z") : Ordered(order) { }

        void activate ();
        void deactivate ();
    };

}

#endif
