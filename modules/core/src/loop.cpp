
#include <stdlib.h>
#include "GL/glfw.h"
#include "../inc/loop.h"


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
        for (;;) {
            glfwPollEvents();
            glfwSleep(1/60.0);
        }
    }

}

