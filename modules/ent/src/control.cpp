#include "ent/inc/control.h"

#include <SDL2/SDL_events.h>
#include "ent/inc/mixins.h"
#include "geo/inc/rooms.h"
#include "hacc/inc/haccable_standard.h"
#include "vis/inc/images.h"

namespace ent {

    bool Mapping::match (SDL_Event* event) {
        switch (event->type) {
            case SDL_KEYUP:
            case SDL_KEYDOWN: {
                switch (type) {
                    case KEY: {
                        return event->key.keysym.sym == code
                            && (!!(event->key.keysym.mod & KMOD_SHIFT) == shift)
                            && (!!(event->key.keysym.mod & KMOD_CTRL) == ctrl)
                            && (!!(event->key.keysym.mod & KMOD_ALT) == alt);
                    }
                    case SCN: return event->key.keysym.scancode == code;
                    case BTN: return false;
                }
            }
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN: {
                switch (type) {
                    case KEY: return false;
                    case SCN: return false;
                    case BTN: return event->button.button == code;
                }
            }
            default: return false;
        }
    }

    Player* player = NULL;

    void Player::Drawn_draw (vis::Overlay) {
        if (!character) return;
        if (cursor_tex && cursor_frame) {
            Vec focus = character->Controllable_get_focus();
            if (defined(focus))
                vis::draw_frame(cursor_frame, cursor_tex, focus);
        }
    }
    void Player::Drawn_draw (vis::Hud) {
        if (!character || !heart_tex || !heart_layout) return;
        if (auto d = dynamic_cast<Damagable*>(character)) {
            int32 life = d->Damagable_life();
            int32 max_life = d->Damagable_max_life();
            for (int i = 0; i < max_life / 48; i++) {
                auto frame = &heart_layout->frames[
                    life >= i * 48 + 48 ? 4
                  : life >= i * 48 + 36 ? 3
                  : life >= i * 48 + 24 ? 2
                  : life >= i * 48 + 12 ? 1
                  :                       0
                ];
                vis::draw_frame(frame, heart_tex, Vec(19.5 - i * 0.75, 14.5));
            }
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
                Vec mot = vis::camera->window_motion_to_world(event->motion.xrel, event->motion.yrel);
                character->Controllable_move_focus(mot);
                return true;
            }
            case SDL_KEYUP:
            case SDL_KEYDOWN: {
                bool found = false;
                size_t bit = 1;
                for (size_t i = 0; i < N_BUTTONS; i++) {
                    for (auto& m : mappings[Button(i)]) {
                        if (m.match(event)) {
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
                        if (m.match(event)) {
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
    void Player::Mind_think_after () {
        if (!character) return;
        camera.size = Vec(20, 15);
        Vec v_pos = character->Controllable_get_vision_pos();
        if (defined(v_pos))
            camera.pos = v_pos;
        Observer::set_room(character->Controllable_get_room());
    }

    Player::Player () {
         // Default mappings
        mappings[BTN_LEFT].emplace_back(SCN, SDL_SCANCODE_A);
        mappings[BTN_RIGHT].emplace_back(SCN, SDL_SCANCODE_D);
        mappings[BTN_DOWN].emplace_back(SCN, SDL_SCANCODE_S);
        mappings[BTN_UP].emplace_back(SCN, SDL_SCANCODE_W);
        mappings[BTN_CROUCH].emplace_back(SCN, SDL_SCANCODE_S);
        mappings[BTN_JUMP].emplace_back(SCN, SDL_SCANCODE_SPACE);
        mappings[BTN_ACTION].emplace_back(SCN, SDL_SCANCODE_E);
        mappings[BTN_EXAMINE].emplace_back(BTN, SDL_BUTTON_LEFT);
        mappings[BTN_AIM].emplace_back(BTN, SDL_BUTTON_RIGHT);
        mappings[BTN_AIM].emplace_back(SCN, SDL_SCANCODE_LSHIFT);
        mappings[BTN_ATTACK].emplace_back(BTN, SDL_BUTTON_LEFT);
        player = this;
        camera.activate();
    }
    Player::~Player () {
        if (player == this)
            player = NULL;
    }

    void Player::finish () {
        if (!active)
            Listener::activate();
        Drawn<vis::Overlay>::appear();
        Drawn<vis::Hud>::appear();
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
    void run_minds_after () {
        for (auto& m : minds()) {
            m.Mind_think_after();
        }
    }

} using namespace ent;

HACCABLE(Controllable) {
    name("ent::Controllable");
}

HACCABLE(Mapping_Type) {
    name("ent::MappingType");
    value("key", KEY);
    value("scn", SCN);
    value("btn", BTN);
}

HACCABLE(Mapping) {
    name("ent::Mapping");
    elem(member(&Mapping::type));
    elem(member(&Mapping::code));
    elem(member(&Mapping::shift).optional());
    elem(member(&Mapping::ctrl).optional());
    elem(member(&Mapping::alt).optional());
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
    attr("Observer", base<geo::Observer>().collapse());
    attr("character", value_methods(&Player::get_character, &Player::set_character));
    attr("cursor_tex", member(&Player::cursor_tex).optional());
    attr("cursor_frame", member(&Player::cursor_frame).optional());
    attr("heart_tex", member(&Player::heart_tex).optional());
    attr("heart_layout", member(&Player::heart_layout).optional());
    attr("mappings", member(&Player::mappings).optional());
    finish(&Player::finish);
}

