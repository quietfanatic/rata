#ifndef HAVE_CORE_GAME_H
#define HAVE_CORE_GAME_H

#include <functional>
#include <vector>
#include "../../util/inc/honestly.h"

namespace core {

    typedef std::function<void ()> Callback;

    extern uint64 frames_simulated;
    extern uint64 frames_drawn;

     // Window is unfortunately a singleton type
    struct Window {
        uint16 width = 640;
        uint16 height = 480;
        uint8 red = 8;
        uint8 green = 8;
        uint8 blue = 8;
        uint8 alpha = 0;
        uint8 depth = 0;
        uint8 stencil = 0;
        bool fullscreen = false;

        Callback step;
        Callback render;

        uint64 frames_simulated = 0;
        uint64 frames_drawn = 0;

        bool is_open = false;
        bool to_stop = false;
        std::vector<std::function<void ()>> pending_ops;

         // Shortcuts.  If you're gonna do multiple of these at once, just
         //  set the parameters and call open() yourself
        void set_size (uint16 w, uint16 h) {
            width = w;
            height = h;
            if (is_open) open();
        }
        void set_width (uint16 w) { width = w; if (is_open) open(); }
        void set_height (uint16 h) { height = h; if (is_open) open(); }
        void set_red (uint8 r) { red = r; if (is_open) open(); }
        void set_green (uint8 g) { green = g; if (is_open) open(); }
        void set_blue (uint8 b) { blue = b; if (is_open) open(); }
        void set_alpha (uint8 a) { alpha = a; if (is_open) open(); }
        void set_depth (uint8 d) { depth = d; if (is_open) open(); }
        void set_stencil (uint8 s) { stencil = s; if (is_open) open(); }
        void set_fullscreen (bool f) { fullscreen = f; if (is_open) open(); }

        Window ();  // Does not open desktop window yet.
        void open ();
        void close ();
        ~Window ();

         // Run the event loop
        void start ();
        void stop ();

         // Run an operation once at the next break between frames, when it
         //  is safe to do things like IO.  Efficiency is not guaranteed.
         // This can be called before start() or even open(), and the op
         //  will be run when the window loop is started.
         // All ops in one frame will be run in one hacc file transaction,
         //  and an exception will abort all ops.
        void before_next_frame (const Callback& f) { pending_ops.emplace_back(f); }
    };
    extern Window* window;

     // Immediately close the window and exit the program.  Does not return.
    void quick_exit ();

}

#endif
