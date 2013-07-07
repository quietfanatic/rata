
#include <GL/glfw.h>
#include "../inc/input.h"
#include "../inc/game.h"
#include "../inc/phases.h"

namespace core {

    std::vector<Key_Listener*> key_listeners;
    std::vector<Char_Listener*> char_listeners;

    static int GLFWCALL close_cb () {
        quick_exit();
        return true;  // not gonna happen
    }

    static void GLFWCALL key_cb (int keycode, int action) {
        for (auto kl : key_listeners) {
            if (kl->hear_key(keycode, action)) return;
        }
        if (action == GLFW_PRESS) {
            switch (keycode) {
                case GLFW_KEY_ESC: {
                    quick_exit();
                    break;
                }
                default: break;
            }
        }
    }
    static void GLFWCALL char_cb (int code, int action) {
        for (auto cl : char_listeners) {
            if (cl->hear_char(code, action)) return;
        }
    }

    struct Input_Phase : core::Phase {
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
    } input_phase;

} using namespace core;

