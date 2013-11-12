#include <GL/glfw.h>
#include "../inc/window.h"
#include "../inc/commands.h"
#include "../inc/input.h"
#include "../../util/inc/debug.h"
#include "../../hacc/inc/files.h"
#include "../../hacc/inc/strings.h"

namespace core {

     // Input handling
    int GLFWCALL close_cb () {
        window->stop();
        return false;
    }
    void GLFWCALL key_cb (int keycode, int action) {
        if (window->key_callback && window->key_callback(keycode, action))
            return;
        if (action == GLFW_PRESS) {
            switch (keycode) {
                case '~': {
                    command_from_terminal();
                    break;
                }
                case GLFW_KEY_ESC: {
                    window->stop();
                    break;
                }
                default: break;
            }
        }
    }
    void GLFWCALL char_cb (int charcode, int action) {
        window->char_callback && window->char_callback(charcode, action);
    }

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
        glfwSetWindowCloseCallback(close_cb);
        glfwSetKeyCallback(key_cb);
        glfwSetCharCallback(char_cb);
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
                        print_to_console("Exception: " + std::string(e.what()) + "\n");
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
HACCABLE(Window) {
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
}

void _load (std::string filename) {
    window->before_next_frame([filename](){ hacc::load(filename); });
};
New_Command _load_cmd ("load", "Manually load a file by its filename.", 1, _load);

void _save (std::string filename) {
    window->before_next_frame([filename](){ hacc::save(filename); });
}
New_Command _save_cmd ("save", "Save the file object with the given filename.", 1, _save);

void _reload (std::string filename) {
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
        window->before_next_frame([filename](){ hacc::reload(filename); });
    }
};
New_Command _reload_cmd ("reload",
    "Reload the file with the given filename, or if none given all files containing '/res/'.",
    1, _reload
);

void _unload (std::string filename) {
    window->before_next_frame([filename](){ hacc::unload(filename); });
};
New_Command _unload_cmd ("unload",
    "Unload the file object with the given filename.  Fails if there are outside references to it.",
    1, _unload
);

void _rename (std::string oldn, std::string newn) {
    hacc::File(oldn).rename(newn);
}
New_Command _rename_cmd ("rename", "Change the filename associated with a file object.", 2, _rename);

void _quit () { core::quick_exit(); }
New_Command _quit_cmd ("quit", "Quit the program immediately without saving anything.", 0, _quit);

void _stop () { window->stop(); }
New_Command _stop_cmd ("stop", "Stop the game (probably saving its state to somewhere)", 0, _stop);

void _files () {
    for (auto f : hacc::loaded_files()) {
        print_to_console(hacc::escape_string(f.filename()));
        print_to_console("\n");
    }
}
New_Command _files_cmd ("files", "List all loaded files", 0, _files);
