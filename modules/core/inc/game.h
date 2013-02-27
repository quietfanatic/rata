#ifndef HAVE_CORE_GAME_H
#define HAVE_CORE_GAME_H

#include "../../util/inc/honestly.h"

namespace core {
    extern uint64 frame_number;
    extern bool initialized;

    void quit_game ();
    void set_video (uint scale);

    void init ();
    void start ();

    void load (std::string filename);
    void save (std::string filename);
}

#endif
