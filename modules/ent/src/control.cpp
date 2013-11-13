#include "../inc/control.h"
#include "../../hacc/inc/haccable_standard.h"
#include <GL/glfw.h>

namespace ent {

    void Player::Drawn_draw (vis::Overlay) {
        if (character && cursor_tex && cursor_frame) {
            Vec focus = character->Controllable_get_focus();
            if (focus.is_defined())
                vis::draw_frame(cursor_frame, cursor_tex, focus);
        }
    }
    void Player::Mind_think () {
        if (!player_controllable) return;
        if (!character) return;
        size_t buttons = 0;
        size_t bit = 1;
        for (size_t i = 0; i < N_BUTTONS; i++) {
            for (auto& m : mappings[Button(i)]) {
                switch (m.type) {
                    case KEY: {
                        if (core::window->key_pressed(m.code))
                            buttons |= bit;
                        break;
                    }
                    case BTN: {
                        if (core::window->btn_pressed(m.code))
                            buttons |= bit;
                        break;
                    }
                    default: break;
                }
            }
            bit <<= 1;
        }
        character->Controllable_buttons(ButtonBits(buttons));
    }
    void Player::Listener_trapped_motion (Vec motion) {
        character->Controllable_move_focus(motion);
    }

    Player::Player () {
         // Default mappings
        mappings[BTN_LEFT].emplace_back(KEY, 'A');
        mappings[BTN_RIGHT].emplace_back(KEY, 'D');
        mappings[BTN_DOWN].emplace_back(KEY, 'S');
        mappings[BTN_UP].emplace_back(KEY, 'W');
        mappings[BTN_CROUCH].emplace_back(KEY, 'S');
        mappings[BTN_JUMP].emplace_back(KEY, GLFW_KEY_SPACE);
        mappings[BTN_ACTION].emplace_back(KEY, 'E');
        mappings[BTN_EXAMINE].emplace_back(BTN, GLFW_MOUSE_BUTTON_LEFT);
        mappings[BTN_AIM].emplace_back(BTN, GLFW_MOUSE_BUTTON_RIGHT);
        mappings[BTN_AIM].emplace_back(KEY, GLFW_KEY_LSHIFT);
        mappings[BTN_ATTACK].emplace_back(BTN, GLFW_MOUSE_BUTTON_LEFT);
    }

    void Player::finish () {
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

HACCABLE(MappingType) {
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
    attr("character", member(&Player::character));
    attr("cursor_tex", member(&Player::cursor_tex).optional());
    attr("cursor_frame", member(&Player::cursor_frame).optional());
    attr("mappings", member(&Player::mappings).optional());
    finish(&Player::finish);
}

