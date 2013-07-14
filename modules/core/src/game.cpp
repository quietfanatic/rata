#include <GL/glfw.h>
#include "../inc/game.h"
#include "../inc/phases.h"
#include "../inc/commands.h"
#include "../../util/inc/debug.h"
#include "../../hacc/inc/files.h"
#include "../../hacc/inc/strings.h"

namespace core {

    Logger file_logger ("files");
    Logger game_logger ("game");

     // Game data
    uint64 frames_simulated = 0;
    uint64 frames_drawn = 0;
     // Scheduled operations
    static std::vector<std::function<void ()>> ops;
    static bool to_stop = false;
    static uint window_scale = 3;

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
        if (scale > 8) scale = 8;
        glfwOpenWindow(
            320*scale, 240*scale,
            8, 8, 8, 0,  // r g b a
            8, 0,  // depth stencil
            GLFW_WINDOW
        );
        window_scale = scale;
    }
    uint get_window_scale () { return window_scale; }
    
    void start (const std::function<void ()>& render) {
        init();
        for (Phase* p : all_phases()) {
            game_logger.log("Starting phase: " + p->name);
            p->Phase_start();
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
                render();
                frames_drawn++;
                glfwSwapBuffers();
                glfwSleep(1/60.0);
            }
            for (Phase* p : all_phases()) p->Phase_stop();
        }
        catch (...) {
            for (Phase* p : all_phases()) p->Phase_stop();
            throw;
        }
    }

} using namespace core;

struct LoadCommand : CommandData {
    std::string filename;
    void operator () () { load(filename); }
};
HCB_BEGIN(LoadCommand)
    new_command<LoadCommand>("load", "Manually load a file by its filename");
    elem(member(&LoadCommand::filename));
HCB_END(LoadCommand)

struct SaveCommand : CommandData {
    std::string filename;
    void operator () () { save(filename); }
};
HCB_BEGIN(SaveCommand)
    new_command<SaveCommand>("save", "Save the file object with the given filename");
    elem(member(&SaveCommand::filename));
HCB_END(SaveCommand)

struct UnloadCommand : CommandData {
    std::string filename;
    void operator () () { unload(filename); }
};
HCB_BEGIN(UnloadCommand)
    new_command<UnloadCommand>("unload", "Unload the file object with the given filename.  Fails if there are outside references to it.");
    elem(member(&UnloadCommand::filename));
HCB_END(SaveCommand)

struct RenameCommand : CommandData {
    std::string old_name;
    std::string new_name;
    void operator () () { hacc::File(old_name).rename(new_name); }
};
HCB_BEGIN(RenameCommand)
    new_command<RenameCommand>("rename", "Change the filename associated with a file object");
    elem(member(&RenameCommand::old_name));
    elem(member(&RenameCommand::new_name));
HCB_END(SaveCommand)

struct ScaleCommand : CommandData {
    uint factor = 1;
    void operator () () { set_video(factor); }
};
HCB_BEGIN(ScaleCommand)
    new_command<ScaleCommand>("scale", "Scale the main window by an integer factor (1 = 320x240, 2 = 640x480)");
    elem(member(&ScaleCommand::factor).optional());
HCB_END(ScaleCommand)

struct QuitCommand : CommandData {
    void operator() () { core::quick_exit(); }
};
HCB_BEGIN(QuitCommand)
    new_command<QuitCommand>("quit", "Quit the program without saving anything");
HCB_END(QuitCommand)

struct StopCommand : CommandData {
    void operator() () { core::stop(); }
};
HCB_BEGIN(StopCommand)
    new_command<StopCommand>("stop", "Stop the game (probably saving its state to somewhere)");
HCB_END(StopCommand)

struct FilesCommand : CommandData {
    void operator() () {
        for (auto f : hacc::loaded_files()) {
            print_to_console(hacc::escape_string(f.filename()));
            print_to_console("\n");
        }
    }
};
HCB_BEGIN(FilesCommand)
    new_command<FilesCommand>("files", "List all loaded files");
HCB_END(FilesCommand)
