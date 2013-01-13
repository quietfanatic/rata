
#include "GL/glfw.h"
#include "../inc/loop.h"

namespace system {
    void set_video () {

    }

    void main_loop () {
        glfwInit();
        glfwOpenWindow(640, 480, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
        glfwSleep(3);
    }

}

