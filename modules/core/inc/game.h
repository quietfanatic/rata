#ifndef HAVE_CORE_GAME_H
#define HAVE_CORE_GAME_H

#include <functional>
#include "../../util/inc/honestly.h"

namespace core {

    extern uint64 frames_simulated;
    extern uint64 frames_drawn;

    void init ();

     // These can be called at any time, and they will
     //  schedule an operation at the next frame break.
     // Loading a new file does not unload any old files.
     //  To do that you must keep track and do it yourself.
    void load (std::string filename);
    void unload (std::string filename);
    void save (std::string filename);
    void set_video (uint scale);
    uint get_window_scale ();
    void start (const std::function<void ()>& render);
    void stop ();
     // Immediately close the window and exit the program.  Does not return.
    void quick_exit ();

}

#endif
