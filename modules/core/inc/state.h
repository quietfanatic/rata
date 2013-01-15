#ifndef HAVE_CORE_STATE_H
#define HAVE_CORE_STATE_H

#include "../../util/inc/organization.h"

namespace core {

struct Stateful;

struct Game_State {
    Links<Stateful> things;
    void exist ();
    ~Game_State () { things.clear(); }
};

extern Game_State* current_state;

struct Stateful : Linkable<Stateful> {
    Stateful () { }

     // Don't register with layers and physics and such until exist() is called.
    virtual void exist () { }
    virtual ~Stateful () { }
};

void load_state (std::string filename);

}


#endif
