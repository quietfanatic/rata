#ifndef HAVE_CORE_GAME_H
#define HAVE_CORE_GAME_H

#include <functional>
#include <vector>
#include "util/inc/honestly.h"

struct SDL_Window;
union SDL_Event;
typedef void* SDL_GLContext;

namespace core {

    struct Listener;

    struct Window {
        uint16 width = 640;
        uint16 height = 480;
        bool fullscreen = false;
        double fps = 60.0;
        bool limit_fps = true;

        std::function<void ()> step;
        std::function<void ()> render;

        Listener* listener = NULL;
        bool cursor_trapped = false;
         // Not updated when trapped.
        int cursor_x = 160;
        int cursor_y = 120;

        uint64 frames_simulated = 0;
        uint64 frames_drawn = 0;

        bool is_open = false;
        bool to_stop = false;
        std::vector<std::function<void ()>> pending_ops;

         // Internal items
        SDL_Window* sdl_window = NULL;
        SDL_GLContext context = NULL;

        Window ();  // Does not open window yet.
        void open ();
        void close ();
        ~Window ();

        void finish () { open(); }

         // Run the event loop
        void start ();
        void stop ();

         // Run an operation once at the next break between frames, when it
         //  is safe to do things like IO.  Efficiency is not guaranteed.
         // This can be called before start() or even open(), and the op
         //  will be run when the window loop is started.
        void before_next_frame (const std::function<void ()>& f) { pending_ops.emplace_back(f); }
    };
    extern Window* window;

     // Interface for input handling.  More recently activated listeners take priority.
    struct Listener {
         // Return true if you've handled the input.
        virtual bool Listener_event (SDL_Event*) { return false; }
         // false = don't trap, true = do trap, -1 = pass
        virtual int Listener_trap_cursor () { return false; }

        bool active = false;
        Listener* next = NULL;
        void activate ();
        void deactivate ();

        virtual ~Listener ();
    };

     // Immediately close the window and exit the program.  Does not return.
    void quick_exit ();

}

#endif
