
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

    void set_video () {

    }

    void main_loop () {
        glfwInit();
        glfwOpenWindow(640, 480, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
        glfwSetKeyCallback(key_cb);
        glfwSetWindowCloseCallback(close_cb);
        vis::Image* test_image = hacc::require_id<vis::Image>("test.png");
        printf("%s\n", test_image->name.c_str());
        for (;;) {
            glfwPollEvents();
            vis::start_draw();
            printf("%s\n", test_image->name.c_str());
            vis::draw_image(test_image, Vec(0, 0), false, false);
            vis::finish_draw();
            glfwSwapBuffers();
            glfwSleep(1/60.0);
        }
    }

}

