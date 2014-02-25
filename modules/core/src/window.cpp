#include <float.h>
#include <GL/glew.h>
#include <GL/glfw.h>
#include "../inc/window.h"
#include "../inc/commands.h"
#include "../../util/inc/debug.h"
#include "../../hacc/inc/files.h"
#include "../../hacc/inc/strings.h"
#include "../../hacc/inc/documents.h"
#include "../../util/inc/integration.h"

namespace core {

     // Input handling
    static int GLFWCALL close_cb () {
        window->stop();
        return false;
    }
    static void GLFWCALL key_cb (int keycode, int action) {
        Listener* next_l = NULL;
        for (Listener* l = window->listener; l; l = next_l) {
             // In case a listener disables itself.
            next_l = l->next;
            if (l->Listener_key(keycode, action))
                return;
        }
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
    static void GLFWCALL char_cb (int charcode, int action) {
        Listener* next_l = NULL;
        for (Listener* l = window->listener; l; l = next_l) {
            next_l = l->next;
            if (l->Listener_char(charcode, action))
                return;
        }
    }
    static void GLFWCALL button_cb (int code, int action) {
        Listener* next_l = NULL;
        for (Listener* l = window->listener; l; l = next_l) {
            next_l = l->next;
            if (l->Listener_button(code, action))
                return;
        }
    }
    static void GLFWCALL resize_cb (int width, int height) {
        window->width = width;
        window->height = height;
    }

    static Logger file_logger ("files");
    static Logger game_logger ("game");
    static Logger timing_logger ("timing", false);


    Window* window = NULL;
    Window::Window () {
        if (window) throw hacc::X::Logic_Error("Tried to create multiple windows at once");
        window = this;
    }
    void Window::open () {
        if (!is_open) {
            auto wd = cwd();
            glfwInit();
            util::chdir(wd);
            is_open = glfwOpenWindow(
                width, height,
                red, green, blue,
                alpha, depth, stencil,
                fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW
            );
            if (!is_open) {
                throw hacc::X::Internal_Error("glfwOpenWindow failed for some reason");
            }
            auto glew_initted = glewInit();
            if (glew_initted != GLEW_OK) {
                throw hacc::X::Internal_Error("GLEW init failed: " + std::string((const char*)glewGetErrorString(glew_initted)));
            }
        }
        else {
            glfwSetWindowSize(width, height);
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
        glfwSetMouseButtonCallback(NULL);
        glfwSetWindowCloseCallback(NULL);
        glfwSetWindowSizeCallback(NULL);
    }

    void Window::start () {
        if (!is_open) open();
         // Set all the window callbacks.  The reason these aren't set on open
         //  is because the input system may crash if the callbacks are called
         //  while the program is exiting and things are being destructed.
        glfwSetWindowCloseCallback(close_cb);
        glfwSetKeyCallback(key_cb);
        glfwSetMouseButtonCallback(button_cb);
        glfwSetCharCallback(char_cb);
        glfwSetWindowSizeCallback(resize_cb);
        glfwDisable(GLFW_AUTO_POLL_EVENTS);
        hacc::set_file_logger([](std::string s){ file_logger.log(s); });
        glfwSetTime(0);
        static double lag = 0;
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
                 // Input handling!
                bool trap_cursor = false;
                Listener* next_l = NULL;
                Listener* l = NULL;
                for (l = window->listener; l; l = next_l) {
                    next_l = l->next;
                    int trap = l->Listener_trap_cursor();
                    if (trap != -1) {
                        trap_cursor = trap;
                        break;
                    }
                }
                if (l == NULL) trap_cursor = false;
                if (trap_cursor != cursor_trapped) {
                    if (trap_cursor) {
                        glfwGetMousePos(&cursor_x, &cursor_y);
                        glfwDisable(GLFW_MOUSE_CURSOR);
                    }
                    else {
                        glfwEnable(GLFW_MOUSE_CURSOR);
                        glfwSetMousePos(cursor_x, cursor_y);
                    }
                }
                cursor_trapped = trap_cursor;
                if (cursor_trapped)
                    glfwSetMousePos(0, 0);
                glfwPollEvents();
                int x, y;
                glfwGetMousePos(&x, &y);
                if (cursor_trapped) {
                    if (l) {
                         // Something's odd about glfw's trapped cursor positioning.
                        if (x != 0 || y != 0)
                            l->Listener_trapped_motion(x+1, y+1);
                        else
                            l->Listener_trapped_motion(0, 0);
                    }
                }
                else {
                    cursor_x = x;
                    cursor_y = y;
                    if (l) {
                        l->Listener_cursor_pos(cursor_x, cursor_y);
                    }
                }
                 // Run step and render
                if (step) step();
                frames_simulated++;
                 // Do timing around the render step.
                 // TODO: this is not quite optimal.
                lag -= 1/fps;
                if (lag > 1/fps + 0.002 && lag < 4/fps) {
                    timing_logger.log("Skipping frame!");
                }
                else {
                    if (lag > 1/fps) {
                         // Allow a tiny bit of slowdown in case the window is
                         //  being vsynced at 59.9hz or something like that.
                        lag = 1/fps;
                    }
                    if (render) render();
                    frames_drawn++;
                }
                lag += glfwGetTime();
                glfwSetTime(0);
                if (limit_fps && lag < 0) {
                    glfwSleep(-lag);
                }
                timing_logger.log("%f\n", lag);
                glfwSwapBuffers();
            }
        }
        catch (...) {
            really_stop_window();
            throw;
        }
        really_stop_window();
    }
    void Window::stop () { to_stop = true; }

    bool Listener::can_poll_input () {
        if (!active) return false;
        Listener* next_l = NULL;
        for (Listener* l = window->listener; l; l = next_l) {
            next_l = l->next;
            if (l == this) return true;
            if (l->Listener_blocks_input_polling()) return false;
        }
        return false;
    }
    bool Listener::key_pressed (int code) { return glfwGetKey(code); }
    bool Listener::btn_pressed (int code) { return glfwGetMouseButton(code); }

    void Listener::activate () {
        if (active) deactivate();
        active = true;
        next = window->listener;
        window->listener = this;
    }
    void Listener::deactivate () {
        for (Listener** l = &window->listener; *l; l = &(*l)->next) {
            if (*l == this) {
                *l = next;
                break;
            }
        }
        next = NULL;
        active = false;
    }

    void quick_exit () {
        glfwTerminate();
        exit(0);
    }

    Listener::~Listener () { deactivate(); }

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
    attr("fps", member(&Window::fps).optional());
    attr("limit_fps", member(&Window::limit_fps).optional());
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
    0, _reload
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

void _create_file (std::string filename, std::string type_name, hacc::Tree data) {
    auto type = hacc::Type(type_name);
    void* p = operator new(type.size());
    type.construct(p);
    hacc::Reference(type, p).from_tree(data);
    hacc::File(filename, hacc::Dynamic(type, p));
}
New_Command _create_file_cmd (
    "create_file", "Create a new file object.  It will not yet be saved to disk.",
    3, _create_file
);

void _add (hacc::Document* doc, std::string type, hacc::Tree data) {
    void* p = doc->alloc(hacc::Type(type));
    hacc::Type(type).construct(p);
    hacc::Reference(hacc::Type(type), p).from_tree(data);
}
New_Command _add_cmd ("add", "Create a new object inside a document.", 2, _add);
void _add_id (hacc::Document* doc, std::string id, std::string type, hacc::Tree data) {
    void* p = doc->alloc_id(id, hacc::Type(type));
    hacc::Type(type).construct(p);
    hacc::Reference(hacc::Type(type), p).from_tree(data);
}
New_Command _add_id_cmd ("add_id", "Create a new object inside a document with a given ID.", 3, _add_id);

void _change_id (hacc::Document* doc, std::string old, std::string new_id) {
    void* p = doc->get(old).address;
    if (!p) throw hacc::X::Logic_Error("This document has no object with ID " + old);
    doc->change_id(p, new_id);
}
New_Command _change_id_cmd (
    "change_id", "Change the ID of an object in a document.  This may break references in unloaded files.",
    3, _change_id
);

void _remove (hacc::Document* doc, std::string id) {
    hacc::Pointer p = doc->get(id);
    if (!p) throw hacc::X::Logic_Error("This document has no object with ID " + id);
    p.type.destruct(p.address);
    doc->dealloc(p.address);
}
New_Command _remove_cmd (
    "remove", "Destroy an object from a document.  This will break any pointers to the object!",
    2, _remove
);

void _window_size (int width, int height) {
    window->set_size(width, height);
}
New_Command _window_size_cmd ("window_size", "Set the window size in pixels.", 2, _window_size);
