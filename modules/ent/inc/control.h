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
        BTN_LEFT,
        BTN_RIGHT,
        BTN_DOWN,
        BTN_UP,
        BTN_CROUCH,
        BTN_JUMP,
        BTN_ACTION,  // Nearby
        BTN_EXAMINE,  // Faraway
        BTN_AIM,
        BTN_ATTACK,
        N_BUTTONS
    };
    enum ButtonBits {
        LEFT_BIT = 1<<BTN_LEFT,
        RIGHT_BIT = 1<<BTN_RIGHT,
        DOWN_BIT = 1<<BTN_DOWN,
        UP_BIT = 1<<BTN_UP,
        CROUCH_BIT = 1<<BTN_CROUCH,
        JUMP_BIT = 1<<BTN_JUMP,
        ACTION_BIT = 1<<BTN_ACTION,
        EXAMINE_BIT = 1<<BTN_EXAMINE,
        AIM_BIT = 1<<BTN_AIM,
        ATTACK_BIT = 1<<BTN_ATTACK
    };

    struct Controllable {
        virtual void control_buttons (ButtonBits) { }
        virtual void move_focus (Vec diff) { }
        virtual Vec get_focus () { return Vec(NAN, NAN); }
        virtual ~Controllable () { }
    };

     // Between input and simulation, these run to do player interaction and AI
    struct Mind : Linkable<Mind> {
        virtual void Mind_think () = 0;
        Mind ();
        ~Mind ();
         // These start out active
        void wake ();
        void sleep ();
    };

     // We're querying key state instead of going through Key_Listener
    struct Player : vis::Sprite, Mind {
        std::vector<int> mappings [N_BUTTONS];  // TODO: mouse buttons
        Controllable* character = NULL;
        void Sprite_draw () override;  // Draws the cursor
        void Mind_think () override;  // Read input and send control to character

        Player ();
        void finish ();
    };

}

#endif
