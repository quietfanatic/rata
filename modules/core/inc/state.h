#ifndef HAVE_CORE_STATE_H
#define HAVE_CORE_STATE_H

#include "../../util/inc/organization.h"

namespace core {

struct Stateful;

struct Game_State {
    Links<Stateful> things;
    bool started = false;

    void start ();
    ~Game_State () { things.destroy_all(); }
};

extern Game_State* current_state;

struct Stateful : Linkable<Stateful> {
    Stateful () { }

     // Don't register with layers and physics and such until exist() is called.
    virtual void start () = 0;
    virtual ~Stateful () { }
};

void load_state (std::string filename);
void save_state (std::string filename);

}


#endif
