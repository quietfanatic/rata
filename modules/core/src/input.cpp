
#include <GL/glfw.h>
#include "../inc/input.h"
#include "../inc/window.h"
#include "../inc/commands.h"

namespace core {

    INIT_SAFE(std::vector<Key_Listener*>, key_listeners);
    INIT_SAFE(std::vector<Char_Listener*>, char_listeners);

    Vec window_cursor_pos = Vec(160, 120);
    Vec cursor_motion = Vec(0, 0);
    bool cursor_trapped = false;
    bool trap_cursor = false;

    bool key_pressed (int code) { return glfwGetKey(code); }
    bool btn_pressed (int code) { return glfwGetMouseButton(code); }

    int GLFWCALL close_cb () {
        quick_exit();
        return true;  // not gonna happen
    }

    void GLFWCALL key_cb (int keycode, int action) {
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
                    window->stop();
                    break;
                }
                default: break;
            }
        }
    }
    void GLFWCALL char_cb (int code, int action) {
        for (auto cl : char_listeners()) {
            if (cl->hear_char(code, action)) return;
        }
    }

     // The ordering of all of these operations is fairly delicate.
    void run_input () {
        if (trap_cursor != cursor_trapped) {
            if (trap_cursor) {
                int x, y;
                glfwGetMousePos(&x, &y);
                window_cursor_pos = Vec(x, y);
                glfwDisable(GLFW_MOUSE_CURSOR);
            }
            else {
                glfwEnable(GLFW_MOUSE_CURSOR);
                glfwSetMousePos(window_cursor_pos.x, window_cursor_pos.y);
            }
        }
        cursor_trapped = trap_cursor;
        if (cursor_trapped)
            glfwSetMousePos(0, 0);
        glfwPollEvents();
        int x, y;
        glfwGetMousePos(&x, &y);
        if (cursor_trapped) {
             // Something's odd about glfw's trapped cursor positioning.
            if (x != 0 || y != 0)
                cursor_motion = Vec(x+1, -(y+1))*PX;
            else
                cursor_motion = Vec(0, 0);
        }
        else {
            window_cursor_pos = Vec(x, y)*PX;
        }
    }

} using namespace core;

