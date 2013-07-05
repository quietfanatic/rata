#ifndef HAVE_CORE_STATE_H
#define HAVE_CORE_STATE_H

#include "../../util/inc/organization.h"

namespace core {

    struct Stateful;
    struct Game_Object;

    struct Game_State {
        std::vector<Game_Object*> pop_culture;
        bool started = false;

        Game_State ();
        void start ();
        ~Game_State ();
    };

    extern Game_State* current_state;

    struct Game_Object {
        virtual void start () = 0;
        virtual ~Game_Object () { }
    };


    bool load_state (std::string filename);
    bool save_state (std::string filename);

     // These are like stateful things in that they belong to a game state,
     //  but they are instantiated automatically when referenced, so they
     //  don't have to be given in the state file.  They pretend to be
     //  global variables.  They must have a nullary constructor.

    uint allocate_celebrity (Game_Object*(*)());
    template <class C> Game_Object* default_celeb_allocator () { return new C; }
    template <class C, Game_Object* (* allocator )() = default_celeb_allocator<C>>
    struct Celebrity {
        uint index = allocate_celebrity(allocator);
        C* get () { return static_cast<C*>(current_state->pop_culture[index]); }
        operator C* () { return get(); }
        C operator * () { return *get(); }
        C* operator -> () { return get(); }
    };

     // Define these with, for instance:
     //  Celebrity<Camera> camera;

    struct Things : Game_Object {
        Links<Stateful> things;
        void start ();
        ~Things() { things.destroy_all(); }
    };
    extern Celebrity<Things> things;

    struct Stateful : Linkable<Stateful>, Game_Object {
        Stateful () { link(things->things); }
        ~Stateful () { unlink(); }
    };

}


#endif
