
#include <GL/glfw.h>
#include "../inc/input.h"
#include "../inc/window.h"
#include "../inc/commands.h"

namespace core {

    Vec window_cursor_pos = Vec(160, 120);
    Vec cursor_motion = Vec(0, 0);
    bool cursor_trapped = false;

    bool key_pressed (int code) { return glfwGetKey(code); }
    bool btn_pressed (int code) { return glfwGetMouseButton(code); }

     // The ordering of all of these operations is fairly delicate.
    void run_input () {
        bool trap_cursor = false;
        Listener* next_l = NULL;
        for (Listener* l = window->listener; l; l = next_l) {
            next_l = l->next;
            int trap = l->Listener_trap_cursor();
            if (trap != -1) {
                trap_cursor = trap;
                break;
            }
        }
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

