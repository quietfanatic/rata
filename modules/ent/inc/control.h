#ifndef HAVE_ENT_CONTROL_H
#define HAVE_ENT_CONTROL_H

#include "core/inc/window.h"
#include "util/inc/geometry.h"
#include "vis/inc/common.h"
namespace vis { struct Texture; struct Frame; }

namespace ent {
    using namespace util;

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
    enum Button_Bits {
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

    enum Mapping_Type {
        KEY,
        BTN
    };

    struct Mapping {
        Mapping_Type type;
        int code;
        Mapping () { }
        Mapping (Mapping_Type type, int code) : type(type), code(code) { }
    };
    struct Mappings {
        std::vector<Mapping> left;
        std::vector<Mapping> right;
        std::vector<Mapping> down;
        std::vector<Mapping> up;
        std::vector<Mapping> crouch;
        std::vector<Mapping> jump;
        std::vector<Mapping> action;
        std::vector<Mapping> examine;
        std::vector<Mapping> aim;
        std::vector<Mapping> attack;
        std::vector<Mapping>& operator [] (Button b) { return *(&left + uint(b)); }
    };

    struct Controllable {
        Controllable** controller = NULL;
        virtual void Controllable_buttons (uint32 bits) { }
         // This changes the focus relative to its current position
        virtual void Controllable_move_focus (Vec diff) { }
         // This should return world coordinates, or NAN,NAN if no focus
        virtual Vec Controllable_get_focus () { return Vec(NAN, NAN); }
        virtual ~Controllable () { if (controller) *controller = NULL; }
    };

     // Between input and simulation, these run to do player interaction and AI
    struct Mind : Link<Mind> {
        virtual void Mind_think () = 0;
        Mind ();
        ~Mind ();
         // These start out active
        void wake ();
        void sleep ();
    };
     // Call think on all minds
    void run_minds ();

     // We're querying key state instead of going through Key_Listener
    struct Player : vis::Drawn<vis::Overlay>, Mind, core::Listener {
        uint32 buttons;
        Mappings mappings;
        Controllable* character = NULL;
        Controllable* get_character () const { return character; }
        void set_character (Controllable* c) {
            if (character) character->controller = NULL;
            character = c;
            c->controller = &character;
        }
        vis::Texture* cursor_tex = NULL;
        vis::Frame* cursor_frame = NULL;

        void Drawn_draw (vis::Overlay) override;  // Draws the cursor
        void Mind_think () override;  // Read input and send control to character

        int Listener_trap_cursor () override { return true; }
        bool Listener_event (SDL_Event*) override;

        Player ();
        ~Player ();
        void finish ();
    };
    extern Player* player;

}

#endif
