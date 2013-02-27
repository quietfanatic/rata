#include <GL/glfw.h>
#include "../inc/game.h"
#include "../inc/phases.h"

namespace core {
    void all_phases (void (PhaseLayer::* method )()) {
        for (PhaseLayer* p : game_phases())
            (p->*method)();
    }
    void all_layers (void (PhaseLayer::* method )()) {
        for (PhaseLayer* p : draw_layers())
            (p->*method)();
    }
    void all_phaselayers (void (PhaseLayer::* method )()) {
        all_phases(method);
        all_layers(method);
    }

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
        all_phaselayers(&PhaseLayer::init);
    }

    void start () {
        try {
            init();
            all_phaselayers(&PhaseLayer::start);
            for (;;) {
                frame_number++;
                all_phases(&PhaseLayer::run_if_on);
                all_layers(&PhaseLayer::run_if_on);
                glfwSwapBuffers();
                glfwSleep(1/60.0);
            }
        } catch (std::exception& e) {
            fprintf(stderr, "Game was aborted due to an exception: %s\n", e.what());
        }
    }

    void stop () {
        all_phaselayers(&PhaseLayer::stop);
    }

}
