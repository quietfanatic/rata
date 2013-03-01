#ifndef HAVE_CORE_STATE_H
#define HAVE_CORE_STATE_H

#include "../../util/inc/organization.h"

namespace core {

    struct Stateful;

    struct Game_State {
        Links<Stateful> things;
        std::vector<void*> pop_culture;
        bool started = false;

        Game_State ();
        void start ();
        ~Game_State ();
    };

    extern Game_State* current_state;

    struct Stateful : Linkable<Stateful> {
        Stateful () { }

         // Don't register with layers and physics and such until exist() is called.
        virtual void start () = 0;
        virtual ~Stateful () { }
    };

    bool load_state (std::string filename);
    bool save_state (std::string filename);

     // These are like stateful things in that they belong to a game state,
     //  but they are instantiated automatically when referenced, so they
     //  don't have to be given in the state file.  They pretend to be
     //  global variables.  They must have a nullary constructor.
    uint allocate_celebrity (void*(*)(), void(*)(void*));
    template <class C> C* default_celeb_allocator () { return new C; }
    template <class C> void default_celeb_deleter (C* p) { delete p; }
    template <class C, C* (* allocator )() = default_celeb_allocator<C>, void (* deleter )(C*) = default_celeb_deleter<C>>
    struct Celebrity {
        uint index = allocate_celebrity((void*(*)())allocator, (void(*)(void*))deleter);
        C* get () { return (C*)current_state->pop_culture[index]; }
        operator C* () { return get(); }
        C operator * () { return *get(); }
        C* operator -> () { return get(); }
    };

     // Define these with, for instance:
     //  Celebrity<Camera> camera;

}


#endif
