#include <GL/glfw.h>
#include "../inc/game.h"
#include "../inc/phases.h"
#include "../../util/inc/debug.h"
#include "../../hacc/inc/files.h"

namespace core {

    Logger file_logger ("files");
    Logger game_logger ("game");

     // Game data
    uint64 frames_simulated = 0;
    uint64 frames_drawn = 0;
     // Scheduled operations
    static std::vector<std::function<void ()>> ops;
    static bool to_stop = false;

    void init () {
        static bool initialized = false;
        if (initialized) return;
        initialized = true;
        hacc::set_file_logger([](std::string s){ file_logger.log(s); });
        glfwInit();
        set_video(3);
    }

    void load (std::string filename) {
        ops.emplace_back([=](){ hacc::load(filename); });
    }
    void unload (std::string filename) {
        ops.emplace_back([=](){ hacc::unload(filename); });
    }
    void save (std::string filename) {
        ops.emplace_back([=](){ hacc::save(filename); });
    }
    void stop () { to_stop = true; }

    void quick_exit () {
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
    
    void start () {
        init();
        game_logger.log("We have %lu phases and %lu layers", all_phases().size(), all_layers().size());
        for (Phase* p : all_phases()) {
            game_logger.log("Starting phase: " + p->name);
            p->Phase_start();
        }
        for (Layer* l : all_layers()) {
            game_logger.log("Starting layer: " + l->name);
            l->Layer_start();
        }
        try {
            for (;;) {
                 // Run queued operations
                if (!ops.empty()) {
                    try {
                        hacc::file_transaction([](){
                            for (auto& o : ops) o();
                        });
                    } catch (std::exception& e) {
                        fprintf(stderr, "Exception: %s\n", e.what());
                    }
                    ops.clear();
                }
                 // Then check for stop
                if (to_stop) {
                    to_stop = false;
                    break;
                }
                 // Run all_phases and all_layers
                 // TODO: real timing and allow frame-skipping the all_layers
                for (Phase* p : all_phases()) p->run_if_on();
                frames_simulated++;
                for (Layer* l : all_layers()) l->run_if_on();
                frames_drawn++;
                glfwSwapBuffers();
                glfwSleep(1/60.0);
            }
            for (Phase* p : all_phases()) p->Phase_stop();
            for (Layer* l : all_layers()) l->Layer_stop();
        }
        catch (...) {
            for (Phase* p : all_phases()) p->Phase_stop();
            for (Layer* l : all_layers()) l->Layer_stop();
            throw;
        }
    }

}
