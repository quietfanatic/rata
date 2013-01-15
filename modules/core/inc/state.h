#ifndef HAVE_CORE_STATE_H
#define HAVE_CORE_STATE_H

#include "../../util/inc/organization.h"

namespace core {

struct Stateful;

struct Game_State {
    Links<Stateful> things;
};

extern Game_State* current_state;

struct Stateful : Linkable<Stateful> {
    Stateful (Game_State* s = current_state) { if (s) link(s->things); }

     // These should do things like register themselves in a phase or layer.
    virtual void freeze () { }
    virtual void thaw () { }
    virtual ~Stateful () { }
};

}


#endif
