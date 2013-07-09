#ifndef HAVE_ENT_CONTROL_H
#define HAVE_ENT_CONTROL_H

#include "../../util/inc/Vec.h"
#include "../../core/inc/input.h"
#include "../../vis/inc/sprites.h"

namespace ent {

     // Buttons are sent from a Controller to a Controllable every frame.
     // If a Controllable does not receive input, it should stop all
     //  voluntary movement.
     // Focus adjustment, however, can be pushed at any time.

    enum Button {
        LEFT,
        RIGHT,
        DOWN,
        UP,
        CROUCH,
        JUMP,
        ACTION,  // Nearby
        EXAMINE,  // Faraway
        AIM,
        ATTACK,
        N_BUTTONS
    };
    enum ButtonBits {
        LEFT_BIT = 1<<LEFT,
        RIGHT_BIT = 1<<RIGHT,
        DOWN_BIT = 1<<DOWN,
        UP_BIT = 1<<UP,
        CROUCH_BIT = 1<<CROUCH,
        JUMP_BIT = 1<<JUMP,
        ACTION_BIT = 1<<ACTION,
        EXAMINE_BIT = 1<<EXAMINE,
        AIM_BIT = 1<<AIM,
        ATTACK_BIT = 1<<ATTACK
    };

    struct Controllable {
        virtual void control_buttons (ButtonBits) { }
        virtual void move_focus (Vec diff) { }
        virtual Vec get_focus () { return Vec(NAN, NAN); }
        virtual ~Controllable () { }
    };

     // Between input and simulation, these run to do player interaction and AI
    struct Mind : Linkable<Mind> {
        virtual void think () = 0;
        Mind ();
        ~Mind ();
         // These start out active
        void activate ();
        void deactivate ();
    };

     // We're querying key state instead of going through Key_Listener
    struct Player : vis::Draws_Sprites, Mind {
        std::vector<int> mappings [N_BUTTONS];  // TODO: mouse buttons
        Controllable* character = NULL;
         // Draws_Sprites
        void draws_sprites ();  // Draws the cursor
         // Mind
        void think ();  // Read input and send control to character

        Player ();
        void finish ();
    };

}

#endif
