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
        virtual void Controllable_buttons (ButtonBits) { }
         // This changes the focus relative to its current position
        virtual void Controllable_move_focus (Vec diff) { }
         // This should return world coordinates, or NAN,NAN if no focus
        virtual Vec Controllable_get_focus () { return Vec(NAN, NAN); }
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
     // TODO: use core::Layer instead of vis::Sprite
    struct Player : vis::Sprite, Mind, core::Cursor_Listener {
        std::vector<int> mappings [N_BUTTONS];  // TODO: mouse buttons
        Controllable* character = NULL;
        core::Texture* cursor_tex = NULL;
        vis::Frame* cursor_frame = NULL;

        void Sprite_draw () override;  // Draws the cursor
        void Mind_think () override;  // Read input and send control to character
        bool Cursor_Listener_active () override;
        bool Cursor_Listener_trap () override;
        void Cursor_Listener_motion (int, int) override;

        Player ();
        void finish ();
    };

}

#endif
