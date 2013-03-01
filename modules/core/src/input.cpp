
#include <GL/glfw.h>
#include "../inc/input.h"
#include "../inc/game.h"
#include "../inc/phases.h"
#include "../inc/state.h"

namespace core {

    std::vector<Key_Listener*> key_listeners;
    std::vector<Char_Listener*> char_listeners;

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
                default: break;
            }
        }
        for (auto kl : key_listeners) {
            if (kl->hear_key(keycode, action)) return;
        }
    }
    static void GLFWCALL char_cb (int code, int action) {
        for (auto cl : char_listeners) {
            if (cl->hear_char(code, action)) return;
        }
    }

    struct Input_Phase : core::Phase, core::Game_Object {
         // Input phase doesn't have a name to keep you from locking out your controls.
        Input_Phase () : core::Phase("A.M") { }
        void start () {
            glfwSetKeyCallback(key_cb);
            glfwSetCharCallback(char_cb);
            glfwSetWindowCloseCallback(close_cb);
            glfwDisable(GLFW_AUTO_POLL_EVENTS);
        }
        void run () {
            glfwPollEvents();
        }
    };
    core::Celebrity<Input_Phase> input_phase;

}

using namespace core;

