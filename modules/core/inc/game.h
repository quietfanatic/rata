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

struct PhaseLayer {
    std::string order;
    std::string name;
    bool on;
    PhaseLayer (std::vector<PhaseLayer*>& type, std::string order, std::string name, bool on);
    virtual void init () { }
    virtual void start () { }
    virtual void run () { }
    virtual void stop () { }
    virtual void quit () { }
    void run_if_on () { if (on) run(); }
    virtual ~PhaseLayer () { }  // Though this should never happen
};

std::vector<PhaseLayer*>& game_phases ();
std::vector<PhaseLayer*>& draw_layers ();

struct Phase : PhaseLayer {
    Phase (std::string order, std::string name = "", bool on = true) :
        PhaseLayer(game_phases(), order, name, on) { }
};
struct Layer : PhaseLayer {
    Layer (std::string order, std::string name = "", bool on = true) :
        PhaseLayer(draw_layers(), order, name, on) { }
};

extern bool initialized;

}

#endif
