#include "../inc/control.h"
#include "../../core/inc/input.h"
#include "../../core/inc/phases.h"
#include "../../hacc/inc/haccable_standard.h"
#include <GL/glfw.h>

namespace ent {

    void Player::Sprite_draw () {
        if (character && cursor_tex && cursor_frame) {
            Vec focus = character->Controllable_get_focus();
            if (focus.is_defined())
                draw_sprite(cursor_frame, cursor_tex, focus);
        }
    }
    void Player::Mind_think () {
        if (!character) return;
        size_t buttons = 0;
        size_t bit = 1;
        for (size_t i = 0; i < N_BUTTONS; i++) {
            for (auto& m : mappings[Button(i)]) {
                switch (m.type) {
                    case KEY: {
                        if (core::key_pressed(m.code))
                            buttons |= bit;
                        break;
                    }
                    case BTN: {
                        if (core::btn_pressed(m.code))
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
    bool Player::Cursor_Listener_active () { return character; }
    bool Player::Cursor_Listener_trap () { return true; }
    void Player::Cursor_Listener_motion (Vec rel) {
        if (!character) return;
        character->Controllable_move_focus(rel);
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
        appear();
    }

    INIT_SAFE(Links<Mind>, minds);
    void Mind::wake () { link(minds()); }
    void Mind::sleep () { unlink(); }
    Mind::Mind () { wake(); }
    Mind::~Mind () { }

    struct Mind_Phase : core::Phase {
        Mind_Phase () : core::Phase("B.M", "minds") { }
        void Phase_run () {
            for (auto m = minds().first(); m; m = m->next()) {
                m->Mind_think();
            }
        }
    } mind_phase;

} using namespace ent;

HCB_BEGIN(Controllable)
    name("ent::Controllable");
HCB_END(Controllable)

HCB_BEGIN(MappingType)
    name("ent::MappingType");
    value("key", KEY);
    value("btn", BTN);
HCB_END(MappingType)

HCB_BEGIN(Mapping)
    name("ent::Mapping");
    elem(member(&Mapping::type));
    elem(member(&Mapping::code));
HCB_END(Mapping)

HCB_BEGIN(Mappings)
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
HCB_END(Mappings)

HCB_BEGIN(Player)
    name("ent::Player");
    attr("character", member(&Player::character));
    attr("cursor_tex", member(&Player::cursor_tex).optional());
    attr("cursor_frame", member(&Player::cursor_frame).optional());
    attr("mappings", member(&Player::mappings).optional());
    finish(&Player::finish);
HCB_END(Player)

