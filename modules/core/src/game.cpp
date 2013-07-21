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

    Window* window = NULL;
    Window::Window () {
        if (window) throw hacc::X::Logic_Error("Tried to create multiple windows at once");
        window = this;
    }
    void Window::open () {
        glfwInit();
        is_open = glfwOpenWindow(
            width, height,
            red, green, blue,
            alpha, depth, stencil,
            fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW
        );
        if (!is_open) {
            throw hacc::X::Internal_Error("glfwOpenWindow failed for some reason");
        }
    }

    void Window::close () {
        is_open = false;
        glfwCloseWindow();
    }
    Window::~Window () {
        close();
        window = NULL;
    }

    void Window::start () {
        if (!is_open) open();
        hacc::set_file_logger([](std::string s){ file_logger.log(s); });
        for (Phase* p : all_phases()) {
            game_logger.log("Starting phase: " + p->name);
            p->Phase_start();
        }
        try {
            for (;;) {
                 // Run queued operations
                if (!pending_ops.empty()) {
                    try {
                        hacc::file_transaction([this](){
                             // Allow ops to be expanded while executing
                            for (size_t i = 0; i < pending_ops.size(); i++) {
                                pending_ops[i]();
                            }
                        });
                    } catch (std::exception& e) {
                        fprintf(stderr, "Exception: %s\n", e.what());
                    }
                    pending_ops.clear();
                }
                 // Then check for stop
                if (to_stop) {
                    to_stop = false;
                    break;
                }
                 // Run all_phases and all_layers
                 // TODO: real timing and allow frame-skipping the all_layers
                for (Phase* p : all_phases()) p->run_if_on();
                if (step) step();
                frames_simulated++;
                if (render) render();
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
    void Window::stop () { to_stop = true; }

    void quick_exit () {
        glfwTerminate();
        exit(0);
    }

} using namespace core;

 // TODO: use magic setters
HCB_BEGIN(Window)
    name("core::Window");
    attr("width", member(&Window::width).optional());
    attr("height", member(&Window::height).optional());
    attr("red", member(&Window::red).optional());
    attr("green", member(&Window::green).optional());
    attr("blue", member(&Window::blue).optional());
    attr("alpha", member(&Window::alpha).optional());
    attr("depth", member(&Window::depth).optional());
    attr("stencil", member(&Window::stencil).optional());
    attr("fullscreen", member(&Window::fullscreen).optional());
HCB_END(Window)

struct LoadCommand : CommandData {
    std::string filename;
    void operator () () { window->before_next_frame([=](){ hacc::load(filename); }); }
};
HCB_BEGIN(LoadCommand)
    new_command<LoadCommand>("load", "Manually load a file by its filename");
    elem(member(&LoadCommand::filename));
HCB_END(LoadCommand)

struct SaveCommand : CommandData {
    std::string filename;
    void operator () () { window->before_next_frame([=](){ hacc::save(filename); }); }
};
HCB_BEGIN(SaveCommand)
    new_command<SaveCommand>("save", "Save the file object with the given filename");
    elem(member(&SaveCommand::filename));
HCB_END(SaveCommand)

struct UnloadCommand : CommandData {
    std::string filename;
    void operator () () { window->before_next_frame([=](){ hacc::unload(filename); }); }
};
HCB_BEGIN(UnloadCommand)
    new_command<UnloadCommand>("unload", "Unload the file object with the given filename.  Fails if there are outside references to it.");
    elem(member(&UnloadCommand::filename));
HCB_END(UnloadCommand)

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

struct QuitCommand : CommandData {
    void operator() () { core::quick_exit(); }
};
HCB_BEGIN(QuitCommand)
    new_command<QuitCommand>("quit", "Quit the program without saving anything");
HCB_END(QuitCommand)

struct StopCommand : CommandData {
    void operator() () { window->stop(); }
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
