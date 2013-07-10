#ifndef HAVE_CORE_INPUT_H
#define HAVE_CORE_INPUT_H

#include <GL/glfw.h>
#include "../../util/inc/honestly.h"
#include "../../util/inc/organization.h"

namespace core {

    bool key_pressed (int keycode);

    struct Key_Listener;
    EXTERN_INIT_SAFE(std::vector<Key_Listener*>, key_listeners)
    struct Key_Listener : Ordered<Key_Listener, key_listeners> {
         // Return true if you've handled the key.
        virtual bool hear_key (int keycode, int action) = 0;

        Key_Listener (std::string order = "Z") : Ordered(order) { }
    };

    struct Char_Listener;
    EXTERN_INIT_SAFE(std::vector<Char_Listener*>, char_listeners)
    struct Char_Listener : Ordered<Char_Listener, char_listeners> {
         // Return true if you've handled the key.
        virtual bool hear_char (int code, int action) = 0;

        Char_Listener (std::string order = "Z") : Ordered(order) { }
    };

    struct Cursor_Listener;
    EXTERN_INIT_SAFE(std::vector<Cursor_Listener*>, cursor_listeners)
    struct Cursor_Listener : Ordered<Cursor_Listener, cursor_listeners> {
         // Return true if you've handled the key.
        virtual bool Cursor_Listener_active () = 0;
        virtual bool Cursor_Listener_trap () { return false; }
        virtual void Cursor_Listener_motion (int x, int y) = 0;

        Cursor_Listener (std::string order = "Z") : Ordered(order) { }
    };

}

#endif
