
#include <GL/glfw.h>
#include "../inc/input.h"
#include "../inc/game.h"
#include "../inc/phases.h"
#include "../inc/commands.h"

namespace core {

    INIT_SAFE(std::vector<Key_Listener*>, key_listeners);
    INIT_SAFE(std::vector<Char_Listener*>, char_listeners);
    INIT_SAFE(std::vector<Cursor_Listener*>, cursor_listeners);

    bool key_pressed (int code) { return glfwGetKey(code); }

    static int GLFWCALL close_cb () {
        quick_exit();
        return true;  // not gonna happen
    }

    static void GLFWCALL key_cb (int keycode, int action) {
        for (auto kl : key_listeners()) {
            if (kl->hear_key(keycode, action)) return;
        }
        if (action == GLFW_PRESS) {
            switch (keycode) {
                case '~': {
                    command_from_terminal();
                    break;
                }
                case GLFW_KEY_ESC: {
                    stop();
                    break;
                }
                default: break;
            }
        }
    }
    static void GLFWCALL char_cb (int code, int action) {
        for (auto cl : char_listeners()) {
            if (cl->hear_char(code, action)) return;
        }
    }
    static void GLFWCALL cursor_cb (int x, int y) {
        for (auto ct : cursor_listeners()) {
            if (ct->Cursor_Listener_active()) {
                ct->Cursor_Listener_motion(x, y);
                break;
            }
        }
    }

    struct Input_Phase : core::Phase {
         // Input phase doesn't have a name to keep you from locking out your controls.
        Input_Phase () : core::Phase("A.M") { }
        void Phase_start () override {
            glfwSetKeyCallback(key_cb);
            glfwSetCharCallback(char_cb);
            glfwSetMousePosCallback(cursor_cb);
            glfwSetWindowCloseCallback(close_cb);
            glfwDisable(GLFW_AUTO_POLL_EVENTS);
        }
         // Not doing this seems to cause surprising happenings,
         //  such as the callbacks being called upon exit() when the
         //  input listeners are partially destructed.
        void Phase_stop () override {
            glfwSetKeyCallback(NULL);
            glfwSetCharCallback(NULL);
            glfwSetMousePosCallback(NULL);
            glfwSetWindowCloseCallback(NULL);
        }
        void Phase_run () override {
            for (auto ct : cursor_listeners()) {
                if (ct->Cursor_Listener_active()) {
                    if (ct->Cursor_Listener_trap()) {
                        glfwDisable(GLFW_MOUSE_CURSOR);
                        glfwSetMousePos(0, 0);
                    }
                    else {
                        glfwEnable(GLFW_MOUSE_CURSOR);
                    }
                    break;
                }
            }
            glfwPollEvents();
        }
    } input_phase;

} using namespace core;

