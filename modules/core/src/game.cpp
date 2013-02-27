#include <GL/glfw.h>
#include "../inc/game.h"
#include "../inc/state.h"
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
    }

    static std::string to_load;
    static std::string to_save;
    void load (std::string filename) { to_load = filename; }
    void save (std::string filename) { to_save = filename; }

    void start () {
        try {
            init();
            for (;;) {
                if (!to_save.empty()) {
                    save_state(to_save);
                    to_save = "";
                }
                if (!to_load.empty()) {
                    bool success = load_state(to_load);
                    to_load = "";
                    if (!success) return;
                }
                frame_number++;
                for (Phase* p : all_phases) p->run_if_on();
                for (Layer* l : all_layers) l->run_if_on();
                glfwSwapBuffers();
                glfwSleep(1/60.0);
            }
        } catch (std::exception& e) {
            fprintf(stderr, "Game was aborted due to an exception: %s\n", e.what());
        }
    }

}
