#include "../inc/control.h"
#include "../../core/inc/input.h"
#include "../../core/inc/phases.h"
#include "../../hacc/inc/haccable_standard.h"
#include <GL/glfw.h>

namespace ent {

    void Player::Sprite_draw () {
         // TODO: draw cursor
    }
    void Player::Mind_think () {
        if (!character) return;
        size_t buttons = 0;
        size_t bit = 1;
        for (size_t i = 0; i < N_BUTTONS; i++) {
            for (auto& k : mappings[i])
                if (core::key_pressed(k))
                    buttons |= bit;
            bit <<= 1;
        }
        character->Controllable_buttons(ButtonBits(buttons));
    }
    bool Player::Cursor_Listener_active () { return character; }
    bool Player::Cursor_Listener_trap () { return true; }
    void Player::Cursor_Listener_motion (int x, int y) {
        if (!character) return;
        character->Controllable_move_focus(Vec(x*PX, y*PX));
    }

    Player::Player () {
         // Default mappings
        mappings[BTN_LEFT].push_back('A');
        mappings[BTN_RIGHT].push_back('D');
        mappings[BTN_DOWN].push_back('S');
        mappings[BTN_UP].push_back('W');
        mappings[BTN_CROUCH].push_back('S');
        mappings[BTN_JUMP].push_back(GLFW_KEY_SPACE);
        mappings[BTN_ACTION].push_back('E');  // TODO: mouse control
        mappings[BTN_EXAMINE].push_back('R');
        mappings[BTN_AIM].push_back(GLFW_KEY_LSHIFT);
        mappings[BTN_ATTACK].push_back('Q');
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

HCB_BEGIN(Player)
    name("ent::Player");
    attr("character", member(&Player::character));
    attr("cursor_tex", member(&Player::cursor_tex).optional());
    attr("map_left", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[BTN_LEFT]; }).optional());
    attr("map_right", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[BTN_RIGHT]; }).optional());
    attr("map_down", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[BTN_DOWN]; }).optional());
    attr("map_up", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[BTN_UP]; }).optional());
    attr("map_crouch", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[BTN_CROUCH]; }).optional());
    attr("map_jump", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[BTN_JUMP]; }).optional());
    attr("map_action", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[BTN_ACTION]; }).optional());
    attr("map_examine", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[BTN_EXAMINE]; }).optional());
    attr("map_aim", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[BTN_AIM]; }).optional());
    attr("map_attack", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[BTN_ATTACK]; }).optional());
    finish(&Player::finish);
HCB_END(Player)

