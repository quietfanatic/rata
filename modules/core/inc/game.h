#ifndef HAVE_CORE_GAME_H
#define HAVE_CORE_GAME_H

#include <string>
#include <vector>

namespace core {

void init ();
void start ();
void stop ();

void quit_game ();
void set_video ();

struct Phase {
    std::string order;
    Phase (std::vector<Phase*>& type, std::string order);
    virtual void init () { }
    virtual void start () { }
    virtual void run () { }
    virtual void stop () { }
    virtual void quit () { }
};

std::vector<Phase*>& game_phases ();
std::vector<Phase*>& draw_phases ();

}

#endif
