#include <GL/glfw.h>
#include "../inc/game.h"
#include "../inc/phases.h"

namespace core {

    uint64 frame_number = 0;

    void quit_game () {
        glfwTerminate();
        exit(0);
    }
    void set_video (uint scale) {
        glfwOpenWindow(
            320*scale, 240*scale,
            8, 8, 8, 0,  // r g b a
            8, 0,  // depth stencil
            GLFW_WINDOW
        );
    }
    
    bool initialized = false;

    void init () {
        if (initialized) return;
        initialized = true;
        glfwInit();
        set_video(3);
        for (PhaseLayer* p : game_phases()) p->init();
        for (PhaseLayer* l : draw_layers()) l->init();
    }

    void start () {
        try {
            init();
            for (PhaseLayer* p : game_phases()) p->start();
            for (PhaseLayer* l : draw_layers()) l->start();
            for (;;) {
                frame_number++;
                for (PhaseLayer* p : game_phases()) p->run_if_on();
                for (PhaseLayer* l : draw_layers()) l->run_if_on();
                glfwSwapBuffers();
                glfwSleep(1/60.0);
            }
        } catch (std::exception& e) {
            fprintf(stderr, "Game was aborted due to an exception: %s\n", e.what());
        }
    }

    void stop () {
        for (PhaseLayer* p : game_phases()) p->stop();
        for (PhaseLayer* l : draw_layers()) l->stop();
    }

}
