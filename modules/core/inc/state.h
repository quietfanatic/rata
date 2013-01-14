#ifndef HAVE_CORE_STATE_H
#define HAVE_CORE_STATE_H

#include "../../util/inc/organization.h"

struct Stateful;

struct Game_State {
    Links<Stateful> things;
};

extern Game_State* current_state;

struct Stateful : Linkable<Stateful> {
    Stateful (Game_State* s = current_state) { if (s) link(s->things); }
    virtual ~Stateful (); // Gotta be polymorphic.
};




#endif
