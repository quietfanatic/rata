
#include <stdlib.h>
#include "GL/glfw.h"

#include "../inc/loop.h"
#include "../inc/commands.h"
#include "../../vis/inc/vis.h"

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

    void main_loop () {
        glfwInit();
        set_video(2);
        glfwSetKeyCallback(key_cb);
        glfwSetWindowCloseCallback(close_cb);
        vis::Image* test_image = hacc::require_id<vis::Image>("vis/test.png");
        for (;;) {
            glfwPollEvents();
            vis::start_draw();
            vis::draw_image(test_image, Vec(0, 0), false, false);
            vis::finish_draw();
            glfwSwapBuffers();
            glfwSleep(1/60.0);
        }
    }

}

