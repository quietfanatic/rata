#include "core/inc/window.h"

#include <float.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <string>
#include "core/inc/commands.h"
#include "hacc/inc/documents.h"
#include "hacc/inc/files.h"
#include "hacc/inc/strings.h"
#include "util/inc/debug.h"
#include "util/inc/integration.h"

using namespace util;

namespace core {

    Window* window = NULL;
    Window::Window () {
        if (window) throw hacc::X::Logic_Error("Tried to create multiple windows at once");
        window = this;
    }
    void Window::open () {
        if (is_open) {
            SDL_SetWindowFullscreen(sdl_window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
            int ox, oy, ow, oh;
            SDL_GetWindowSize(sdl_window, &ow, &oh);
            if (ow != width || oh != height) {
                SDL_GetWindowPosition(sdl_window, &ox, &oy);
                SDL_SetWindowPosition(sdl_window, ox - (width - ow)/2, oy - (height - oh)/2);
                SDL_SetWindowSize(sdl_window, width, height);
            }
        }
        else {
            auto wd = cwd();
            if (SDL_Init(0) < 0) {
                throw hacc::X::Internal_Error("SDL_Init failed: " + std::string(SDL_GetError()));
            }
            if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
                throw hacc::X::Internal_Error("SDL_Init video failed: " + std::string(SDL_GetError()));
            }
            atexit(SDL_Quit);
            util::chdir(wd);
            sdl_window = SDL_CreateWindow("asdfasdf",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                width, height,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
                | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0)
            );
            if (!sdl_window) {
                throw hacc::X::Internal_Error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
            }
            context = SDL_GL_CreateContext(sdl_window);
            if (!context) {
                throw hacc::X::Internal_Error("SDL_GL_CreateContext failed: " + std::string(SDL_GetError()));
            }
            auto glew_initted = glewInit();
            if (glew_initted != GLEW_OK) {
                throw hacc::X::Internal_Error("GLEW init failed: " + std::string((const char*)glewGetErrorString(glew_initted)));
            }
            if (!glewIsSupported("GL_VERSION_2_1 GL_ARB_texture_non_power_of_two GL_ARB_framebuffer_object")) {
                throw hacc::X::Error("This implementation of OpenGL does not support enough things.");
            }
            is_open = true;
        }
    }

    void Window::close () {
        is_open = false;
        SDL_DestroyWindow(sdl_window);
    }
    Window::~Window () {
        close();
        window = NULL;
    }

    void Window::start () {
        if (!is_open) open();
        double lag = 0;
        uint32 last_ticks = SDL_GetTicks();

         // MAIN RENDER LOOP
        for (;;) {
             // Run queued operations
            if (!pending_ops.empty()) {
                try {
                     // Allow ops to be expanded while executing
                    for (size_t i = 0; i < pending_ops.size(); i++) {
                        pending_ops[i]();
                    }
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

             // Input handling
             // Decide whether to trap the cursor
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
                SDL_SetRelativeMouseMode(SDL_bool(trap_cursor));
            }
            cursor_trapped = trap_cursor;
             // SDL-style event loop
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                Listener* next_l = NULL;
                for (Listener* l = window->listener; l; l = next_l) {
                     // In case a listener disables itself.
                    next_l = l->next;
                    if (l->Listener_event(&event))
                        goto next_event;
                }
                switch (event.type) {
                    case SDL_QUIT: {
                        stop();
                    }
                    case SDL_KEYDOWN: {
                        switch (event.key.keysym.scancode) {
                            case 0x35: {  // `
                                command_from_terminal();
                                break;
                            }
                            case 0x29: { // Escape
                                window->stop();
                                break;
                            }
                            default: break;
                        }
                        break;
                    }
                    case SDL_WINDOWEVENT: {
                        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                            width = event.window.data1;
                            height = event.window.data2;
                        }
                        break;
                    }
                }
                next_event: { }
            }

             // Run step and render
            if (step) step();
            frames_simulated++;
            logging_frame = frames_simulated;
             // Do timing around the render step.
             // TODO: this is not quite optimal.
            lag -= 1/fps;
            if (!limit_fps && lag > 1/fps + 0.002 && lag < 4/fps) {
                log("frameskip", "Skipping frame!");
            }
            else {
                if (lag > 1/fps) {
                     // Allow a tiny bit of slowdown in case the window is
                     //  being vsynced at 59.9hz or something like that.
                    lag = 1/fps;
                }
                if (render) render();
                frames_drawn++;
                uint32 new_ticks = SDL_GetTicks();
                lag += (new_ticks - last_ticks) / 1000.0;
                last_ticks = new_ticks;
                if (limit_fps && lag < 0) {
                    SDL_Delay(-lag * 1000);
                }
                log("timing", "%f", lag);
                SDL_GL_SwapWindow(sdl_window);
            }
        }
    }
    void Window::stop () { to_stop = true; }

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
        exit(0);
    }

    Listener::~Listener () { deactivate(); }

} using namespace core;

 // TODO: use magic setters
HACCABLE(Window) {
    name("core::Window");
    attr("width", member(&Window::width).optional());
    attr("height", member(&Window::height).optional());
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
            hacc::file_transaction([](){
                for (auto f : hacc::loaded_files()) {
                    if (f.filename().find("/res/") != std::string::npos
                     || f.filename().find("world/") != std::string::npos) {
                        hacc::reload(f);
                    }
                }
            });
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

void _create_file (std::string filename, hacc::Tree data) {
    hacc::Dynamic d;
    hacc::Reference(&d).from_tree(data);
    hacc::File(filename, std::move(d));
}
New_Command _create_file_cmd (
    "create_file", "Create a new file object.  It will not yet be saved to disk.",
    2, _create_file
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
    window->width = width;
    window->height = height;
    window->open();
}
New_Command _window_size_cmd ("window_size", "Set the window size in pixels.", 2, _window_size);

void _window_fullscreen () {
    window->fullscreen = !window->fullscreen;
    window->open();
}
New_Command _window_fullscreen_cmd ("window_fullscreen", "Toggle between window fullscreen modes", 0, _window_fullscreen);
