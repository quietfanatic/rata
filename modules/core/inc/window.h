#ifndef HAVE_CORE_GAME_H
#define HAVE_CORE_GAME_H

#include <functional>
#include <vector>
#include "../../util/inc/honestly.h"

namespace core {

    extern uint64 frames_simulated;
    extern uint64 frames_drawn;

     // Window is unfortunately a singleton type.  If we ever support
     //  GLFW 3, we can fix that.
    struct Window {
        uint16 width = 640;
        uint16 height = 480;
         // Bit depths
        uint8 red = 8;
        uint8 green = 8;
        uint8 blue = 8;
        uint8 alpha = 0;
        uint8 depth = 0;
        uint8 stencil = 0;
        bool fullscreen = false;
        bool paused = false;

        std::function<void ()> step;
        std::function<void ()> render;
         // Return true if you've handled the key
        std::function<bool (int, int)> key_callback;
        std::function<bool (int, int)> char_callback;

        uint64 frames_simulated = 0;
        uint64 frames_drawn = 0;

        bool is_open = false;
        bool to_stop = false;
        std::vector<std::function<void ()>> pending_ops;

         // Shortcuts.  If you want to set any other fields, do it manually and
         //  then call open().
        void set_size (uint16 w, uint16 h) {
            width = w;
            height = h;
            if (is_open) open();
        }
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
        void before_next_frame (const std::function<void ()>& f) { pending_ops.emplace_back(f); }
    };
    extern Window* window;

     // Immediately close the window and exit the program.  Does not return.
    void quick_exit ();

}

#endif
