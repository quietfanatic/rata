#ifndef HAVE_CORE_INPUT_H
#define HAVE_CORE_INPUT_H

#include <GL/glfw.h>
#include "../../util/inc/honestly.h"
#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"

namespace core {
    using namespace util;

    bool key_pressed (int keycode);
    bool btn_pressed (int code);
     // If true, hide desktop pointer and commandeer the cursor.
    extern bool trap_cursor;
     // This is only updated when the cursor is not trapped.
    extern Vec window_cursor_pos;
     // Motion this frame.  This is 0,0 if the cursor is not trapped.
    extern Vec cursor_motion;

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

    int GLFWCALL close_cb ();
    void GLFWCALL key_cb (int, int);
    void GLFWCALL char_cb (int, int);
    void run_input ();

}

#endif
