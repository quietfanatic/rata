#ifndef HAVE_CORE_LOOP_H
#define HAVE_CORE_LOOP_H

#include <string>
#include <vector>

namespace core {

void quit_game ();
void set_video ();
void main_loop ();

struct Phase {
    std::string order;
    Phase (std::vector<Phase*>& type, std::string order);
    virtual void run () = 0;
};

std::vector<Phase*>& game_phases ();
std::vector<Phase*>& draw_phases ();

}

#endif
