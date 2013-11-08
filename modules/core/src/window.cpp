#include <GL/glfw.h>
#include "../inc/window.h"
#include "../inc/commands.h"
#include "../inc/input.h"
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

    void really_stop_window () {
        glfwSetKeyCallback(NULL);
        glfwSetCharCallback(NULL);
        glfwSetWindowCloseCallback(NULL);
    }

    void Window::start () {
        if (!is_open) open();
         // Set all the window calbacks.  The reason these aren't set on open
         //  is because the input system may crash if the callbacks are called
         //  while the program is exiting and things are being destructed.
        glfwSetKeyCallback(key_cb);
        glfwSetCharCallback(char_cb);
        glfwSetWindowCloseCallback(close_cb);
        glfwDisable(GLFW_AUTO_POLL_EVENTS);
        hacc::set_file_logger([](std::string s){ file_logger.log(s); });
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
                run_input();
                 // Run step and render
                 // TODO: real timing and allow frame-skipping the all_layers
                if (step) step();
                frames_simulated++;
                if (render) render();
                frames_drawn++;
                glfwSwapBuffers();
                glfwSleep(1/60.0);
            }
        }
        catch (...) {
            really_stop_window();
            throw;
        }
        really_stop_window();
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
    void operator () () {
        auto f = filename;  // *this is deallocated before this lambda is run
        window->before_next_frame([f](){ hacc::load(f); });
    }
};
HCB_BEGIN(LoadCommand)
    new_command<LoadCommand>("load", "Manually load a file by its filename");
    elem(member(&LoadCommand::filename));
HCB_END(LoadCommand)

struct SaveCommand : CommandData {
    std::string filename;
    void operator () () {
        auto f = filename;
        window->before_next_frame([f](){ hacc::save(f); });
    }
};
HCB_BEGIN(SaveCommand)
    new_command<SaveCommand>("save", "Save the file object with the given filename");
    elem(member(&SaveCommand::filename));
HCB_END(SaveCommand)

struct ReloadCommand : CommandData {
    std::string filename;
    void operator () () {
        if (filename.empty()) {
            window->before_next_frame([](){
                for (auto f : hacc::loaded_files()) {
                    if (f.filename().find("/res/") != std::string::npos) {
                        hacc::reload(f);
                    }
                }
            });
        }
        else {
            auto f = filename;
            window->before_next_frame([f](){ hacc::reload(f); });
        }
    }
};
HCB_BEGIN(ReloadCommand)
    new_command<ReloadCommand>("reload", "Reload the file with the given filename, or if none given all files containing '/res/'");
    elem(member(&ReloadCommand::filename).optional());
HCB_END(ReloadCommand);

struct UnloadCommand : CommandData {
    std::string filename;
    void operator () () {
        auto f = filename;
        window->before_next_frame([f](){ hacc::unload(f); });
    }
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
