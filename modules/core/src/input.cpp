
#include <GL/glfw.h>
#include "../inc/input.h"
#include "../inc/commands.h"
#include "../inc/game.h"
#include "../inc/phases.h"
#include "../inc/state.h"

namespace core {

    std::vector<Key_Listener*> key_listeners;

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
                    command_from_terminal();
                    break;
                }
                default: break;
            }
        }
        for (auto kl : key_listeners) {
            kl->hear_key(keycode, action);
        }
    }

    struct Input_Phase : core::Phase, core::Stateful {
         // Input phase doesn't have a name to keep you from locking out your controls.
        Input_Phase () : core::Phase("A.M") { }
        void start () {
            glfwSetKeyCallback(key_cb);
            glfwSetWindowCloseCallback(close_cb);
            glfwDisable(GLFW_AUTO_POLL_EVENTS);
        }
        void run () {
            glfwPollEvents();
        }
    };

}

using namespace core;

HCB_BEGIN(Input_Phase)
    type_name("core::Input_Phase");
    base<Stateful>("Input_Phase");
    empty();
HCB_END(Input_Phase)

