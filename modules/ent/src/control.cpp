#include "../inc/control.h"
#include "../../core/inc/input.h"
#include "../../core/inc/phases.h"
#include "../../hacc/inc/haccable_standard.h"
#include <GL/glfw.h>

namespace ent {

    void Player::Sprite_draw () {
         // TODO: draw cursor
    }
    void Player::think () {
        if (!character) return;
        size_t buttons = 0;
        size_t bit = 1;
        for (size_t i = 0; i < N_BUTTONS; i++) {
            for (auto& k : mappings[i])
                if (core::key_pressed(k))
                    buttons |= bit;
            bit <<= 1;
        }
        character->control_buttons(ButtonBits(buttons));
    }

    Player::Player () {
         // Default mappings
        mappings[LEFT].push_back('A');
        mappings[RIGHT].push_back('D');
        mappings[DOWN].push_back('S');
        mappings[UP].push_back('W');
        mappings[CROUCH].push_back('S');
        mappings[JUMP].push_back(GLFW_KEY_SPACE);
        mappings[ACTION].push_back('E');  // TODO: mouse control
        mappings[EXAMINE].push_back('R');
        mappings[AIM].push_back(GLFW_KEY_LSHIFT);
        mappings[ATTACK].push_back('Q');
    }

    void Player::finish () {
        appear();
    }

    INIT_SAFE(Links<Mind>, minds);
    void Mind::activate () { link(minds()); }
    void Mind::deactivate () { unlink(); }
    Mind::Mind () { activate(); }
    Mind::~Mind () { }

    struct Mind_Phase : core::Phase {
        Mind_Phase () : core::Phase("B.M", "minds") { }
        void Phase_run () {
            for (Mind* m = minds().first(); m; m = m->next()) {
                m->think();
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
    attr("map_left", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[LEFT]; }).optional());
    attr("map_right", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[RIGHT]; }).optional());
    attr("map_down", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[DOWN]; }).optional());
    attr("map_up", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[UP]; }).optional());
    attr("map_crouch", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[CROUCH]; }).optional());
    attr("map_jump", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[JUMP]; }).optional());
    attr("map_action", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[ACTION]; }).optional());
    attr("map_examine", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[EXAMINE]; }).optional());
    attr("map_aim", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[AIM]; }).optional());
    attr("map_attack", ref_func<std::vector<int>>([](Player& p)->std::vector<int>&{ return p.mappings[ATTACK]; }).optional());
    finish(&Player::finish);
HCB_END(Player)

