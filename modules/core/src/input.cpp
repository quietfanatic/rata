
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
    bool btn_pressed (int code) { return glfwGetMouseButton(code); }

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

    struct Input_Phase : core::Phase {
        bool cursor_trapped = false;
        int cursor_old_x = 160;  // These values probably don't really matter
        int cursor_old_y = 120;
         // Input phase doesn't have a name to keep you from locking out your controls.
        Input_Phase () : core::Phase("A.M") { }
        void Phase_start () override {
            glfwSetKeyCallback(key_cb);
            glfwSetCharCallback(char_cb);
            glfwSetWindowCloseCallback(close_cb);
            glfwDisable(GLFW_AUTO_POLL_EVENTS);
        }
        void Phase_stop () override {
             // Not doing this seems to cause surprising happenings,
             //  such as the callbacks being called upon exit() when the
             //  input listeners are partially destructed.
            glfwSetKeyCallback(NULL);
            glfwSetCharCallback(NULL);
            glfwSetWindowCloseCallback(NULL);
        }
        void Phase_run () override {
            for (auto ct : cursor_listeners()) {
                if (ct->Cursor_Listener_active()) {
                    if (ct->Cursor_Listener_trap() != cursor_trapped) {
                        cursor_trapped = !cursor_trapped;
                        if (cursor_trapped) {
                            glfwGetMousePos(&cursor_old_x, &cursor_old_y);
                            glfwDisable(GLFW_MOUSE_CURSOR);
                        }
                        else {
                            glfwEnable(GLFW_MOUSE_CURSOR);
                            glfwSetMousePos(cursor_old_x, cursor_old_y);
                        }
                    }
                    break;
                }
            }
            if (cursor_trapped)
                glfwSetMousePos(0, 0);
            glfwPollEvents();
            int x, y;
            glfwGetMousePos(&x, &y);
            if (!cursor_trapped || (x != 0 && y != 0)) {
                for (auto ct : cursor_listeners()) {
                    if (ct->Cursor_Listener_active()) {
                        ct->Cursor_Listener_motion(Vec(x+1, -(y+1))*PX);
                        break;
                    }
                }
            }
        }
    } input_phase;

} using namespace core;

