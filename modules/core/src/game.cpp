
#include <stdlib.h>
#include "GL/glfw.h"

#include "../inc/game.h"
#include "../inc/state.h"
#include "../inc/commands.h"
#include "../../hacc/inc/everything.h"

namespace core {

    void quit_game () {
        glfwTerminate();
        exit(0);
    }

    int GLFWCALL close_cb () {
        quit_game();
        return true;  // not gonna happen
    }

    void GLFWCALL key_cb (int keycode, int action) {
        if (action == GLFW_PRESS) {
            switch (keycode) {
                case GLFW_KEY_ESC: {
                    quit_game();
                }
                case '`': {
                    command_from_terminal();
                }
                default: {
//                    if (keycode < 400)
  //                      key[keycode] = 1;
    //                    return;
                }
            }
        }
        else { // action == GLFW_RELEASE
      //      if (keycode < 400)
        //        key[keycode] = 0;
          //      return;
        }
    }

    void set_video (uint scale) {
        glfwOpenWindow(320*scale, 240*scale, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
    }

    void init () {
        glfwInit();
        set_video(2);
        glfwSetKeyCallback(key_cb);
        glfwSetWindowCloseCallback(close_cb);
    }

    void play () {
        if (!current_state) {
            fprintf(stderr, "Error: cannot play game because a game state was not loaded.\n");
            return;
        }
        for (;;) {
            glfwPollEvents();
            for (Phase* p : game_phases())
                p->run();
            for (Phase* p : draw_phases())
                p->run();
            glfwSwapBuffers();
            glfwSleep(1/60.0);
        }
    }

    void load (std::string filename) {
        load_state(filename);
    }

    Phase::Phase (std::vector<Phase*>& type, std::string order) : order(order) {
        for (auto i = type.begin(); i != type.end(); i++) {
            if (order < (*i)->order) {
                type.insert(i, this);
                return;
            }
        }
        type.push_back(this);
    }

    std::vector<Phase*>& game_phases () { static std::vector<Phase*> r; return r; }
    std::vector<Phase*>& draw_phases () { static std::vector<Phase*> r; return r; }

}

