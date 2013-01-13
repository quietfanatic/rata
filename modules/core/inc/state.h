#ifndef HAVE_SYSTEM_STATE_H
#define HAVE_SYSTEM_STATE_H

struct Stateful;

struct Game_State {
    Links<Stateful> all_the_things;
};

extern Game_State* current_state;

struct Stateful : Linkable<Stateful> {
    Stateful (Game_State* s = current_state) { if (s) link(s->all_the_things); }
    virtual ~Stateful (); // Gotta be polymorphic.
};




#endif
