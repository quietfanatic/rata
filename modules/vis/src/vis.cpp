#include "GL/gl.h"

#include "../inc/vis.h"

namespace vis {

    void start_draw () {
        glClearColor(0.5, 0.5, 0.5, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void finish_draw () {
         // Leave flipping to core, so we don't have to include glfw
    }

}

