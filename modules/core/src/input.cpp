
#include <GL/glfw.h>
#include "../inc/input.h"
#include "../inc/game.h"
#include "../inc/commands.h"

namespace core {

    static const uint NUM_KEYS = 400;

    static uint8 keys [NUM_KEYS];

    static void register_keypress (uint code) {
        if (code < NUM_KEYS) keys[code] = 1;
    }
    static void register_keyrelease (uint code) {
        if (code < NUM_KEYS) keys[code] = 0;
    }
    uint8 get_key (uint code) {
        return code < NUM_KEYS ? keys[code] : 0;
    }

    static int GLFWCALL close_cb () {
        quit_game();
        return true;  // not gonna happen
    }


    static void GLFWCALL key_cb (int keycode, int action) {
        if (action == GLFW_PRESS) {
            switch (keycode) {
                case GLFW_KEY_ESC: {
                    quit_game();
                    break;
                }
                case '`': {
                    enter_console();
                    break;
                }
                default: break;
            }
            register_keypress(keycode);
        }
        else { // action == GLFW_RELEASE
            register_keyrelease(keycode);
        }
    }

    void temp_key_cb (void GLFWCALL (* cb ) (int keycode, int action)) {
        glfwSetKeyCallback(cb);
    }
    void undo_temp_key_cb () {
        glfwSetKeyCallback(key_cb);
    }

    struct Input_Phase : core::Phase {
         // Input phase doesn't have a name to keep you from locking out your controls.
        Input_Phase () : core::Phase("A.M") { }
        void init () {
            for (uint i = 0; i < NUM_KEYS; i++)
                keys[i] = 0;
            glfwSetKeyCallback(key_cb);
            glfwSetWindowCloseCallback(close_cb);
            glfwDisable(GLFW_AUTO_POLL_EVENTS);
        }
        void run () {
            for (uint i = 0; i < NUM_KEYS; i++)
                if (keys[i] && keys[i] != 255)
                    keys[i]++;
            glfwPollEvents();
        }
    } input_phase;

}

