#include "ent/inc/control.h"

#include <SDL2/SDL_events.h>
#include "geo/inc/camera.h"
#include "geo/inc/rooms.h"
#include "hacc/inc/haccable_standard.h"
#include "vis/inc/images.h"

namespace ent {

    Player* player = NULL;

    void Player::Drawn_draw (vis::Overlay) {
        if (character && cursor_tex && cursor_frame) {
            Vec focus = character->Controllable_get_focus();
            if (focus.is_defined())
                vis::draw_frame(cursor_frame, cursor_tex, focus);
        }
    }
    void Player::Mind_think () {
        if (!character) return;
        character->Controllable_buttons(buttons);
    }
    bool Player::Listener_event (SDL_Event* event) {
        if (!character) return false;
        switch (event->type) {
            case SDL_MOUSEMOTION: {
                Vec mot = geo::camera->window_motion_to_world(event->motion.xrel, event->motion.yrel);
                character->Controllable_move_focus(mot);
                return true;
            }
            case SDL_KEYUP:
            case SDL_KEYDOWN: {
                bool found = false;
                size_t bit = 1;
                for (size_t i = 0; i < N_BUTTONS; i++) {
                    for (auto& m : mappings[Button(i)]) {
                        if (m.type == KEY && m.code == event->key.keysym.scancode) {
                            if (event->key.state)
                                buttons |= bit;
                            else
                                buttons &= ~bit;
                            found = true;
                        }
                    }
                    bit <<= 1;
                }
                return found;
            }
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN: {
                bool found = false;
                size_t bit = 1;
                for (size_t i = 0; i < N_BUTTONS; i++) {
                    for (auto& m : mappings[Button(i)]) {
                        if (m.type == KEY && m.code == event->button.button) {
                            if (event->button.state)
                                buttons |= bit;
                            else
                                buttons &= ~bit;
                            found = true;
                        }
                    }
                    bit <<= 1;
                }
                return found;
            }
            case SDL_WINDOWEVENT: {
                if (event->window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    buttons = 0;
                }
                return false;  // Don't hog this event
            }
            default: return false;
        }
    }

    Player::Player () {
         // Default mappings
        mappings[BTN_LEFT].emplace_back(KEY, SDL_SCANCODE_A);
        mappings[BTN_RIGHT].emplace_back(KEY, SDL_SCANCODE_D);
        mappings[BTN_DOWN].emplace_back(KEY, SDL_SCANCODE_S);
        mappings[BTN_UP].emplace_back(KEY, SDL_SCANCODE_W);
        mappings[BTN_CROUCH].emplace_back(KEY, SDL_SCANCODE_S);
        mappings[BTN_JUMP].emplace_back(KEY, SDL_SCANCODE_SPACE);
        mappings[BTN_ACTION].emplace_back(KEY, SDL_SCANCODE_E);
        mappings[BTN_EXAMINE].emplace_back(BTN, SDL_BUTTON_LEFT);
        mappings[BTN_AIM].emplace_back(BTN, SDL_BUTTON_RIGHT);
        mappings[BTN_AIM].emplace_back(KEY, SDL_SCANCODE_LSHIFT);
        mappings[BTN_ATTACK].emplace_back(BTN, SDL_BUTTON_LEFT);
        player = this;
    }
    Player::~Player () {
        if (player == this)
            player = NULL;
    }

    void Player::finish () {
        if (!active)
            Listener::activate();
        appear();
    }

    bool player_controllable = true;

    static INIT_SAFE(Links<Mind>, minds);
    void Mind::wake () { link(minds()); }
    void Mind::sleep () { unlink(); }
    Mind::Mind () { wake(); }
    Mind::~Mind () { }

    void run_minds () {
        for (auto& m : minds()) {
            m.Mind_think();
        }
    }

} using namespace ent;

HACCABLE(Controllable) {
    name("ent::Controllable");
}

HACCABLE(Mapping_Type) {
    name("ent::MappingType");
    value("key", KEY);
    value("btn", BTN);
}

HACCABLE(Mapping) {
    name("ent::Mapping");
    elem(member(&Mapping::type));
    elem(member(&Mapping::code));
}

HACCABLE(Mappings) {
    name("ent::Mappings");
    attr("left", member(&Mappings::left).optional());
    attr("right", member(&Mappings::right).optional());
    attr("down", member(&Mappings::down).optional());
    attr("up", member(&Mappings::up).optional());
    attr("crouch", member(&Mappings::crouch).optional());
    attr("jump", member(&Mappings::jump).optional());
    attr("action", member(&Mappings::action).optional());
    attr("examine", member(&Mappings::up).optional());
    attr("aim", member(&Mappings::aim).optional());
    attr("attack", member(&Mappings::up).optional());
}

HACCABLE(Player) {
    name("ent::Player");
    attr("character", value_methods(&Player::get_character, &Player::set_character));
    attr("cursor_tex", member(&Player::cursor_tex).optional());
    attr("cursor_frame", member(&Player::cursor_frame).optional());
    attr("mappings", member(&Player::mappings).optional());
    finish(&Player::finish);
}

