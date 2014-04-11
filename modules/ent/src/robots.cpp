#include "ent/inc/bipeds.h"
#include "ent/inc/bullets.h"
#include "ent/inc/control.h"
#include "ent/inc/mixins.h"
#include "geo/inc/vision.h"
#include "hacc/inc/everything.h"
#include "phys/inc/ground.h"
#include "shell/inc/main.h"
#include "util/inc/debug.h"
#include "util/inc/geometry.h"
#include "vis/inc/images.h"
using namespace phys;
using namespace util;
using namespace vis;

namespace ent {

    struct Ext_Def : Object_Def {
        vis::Texture* texture;
        vis::Layout* layout;
    };

     // Robots cannot see through walls.
    struct Robot : ROD<Sprites, Ext_Def>, Controllable {
         // This is not stored.  TODO: should it be?
        Object* enemy = NULL;
        Vec enemy_pos = Vec(0, 0);
        int8 direction = 0;

        uint32 buttons = 0;
        Vec focus = Vec(0, 0);
        static CE Vec focus_offset () { return Vec(0, 0.5); }
        Vec vision_pos;
        geo::Vision vision;
        void Controllable_buttons (uint32 bits) override {
            if (bits)
                b2body->SetAwake(true);
            buttons = bits;
        }
        void Controllable_move_focus (Vec diff) override {
            focus = constrain(Rect(-18, -13, 18, 13), focus + diff);
        }
        Vec Controllable_get_focus () override {
            return focus + get_pos() + Vec(0, 0.5);
        }
        Vec Controllable_get_vision_pos () override { return vision_pos; }
        geo::Room* Controllable_get_room () { return room; }

         // All this does is do some ray casts to make sure we have a direct
         //  line of sight to the human.  TODO: check Filters instead of dynamic cast to Biped
        virtual void Object_before_move () override {
            auto old_enemy = enemy;
            enemy = NULL;
            foreach_contact([&](b2Fixture* mine, b2Fixture* other){
                auto fd = (phys::FixtureDef*)mine->GetUserData();
                auto oo = (Object*)other->GetBody()->GetUserData();
                if (dynamic_cast<Biped*>(oo)) {
                    if (!enemy || length2(oo->get_pos() - get_pos()) < length2(enemy->get_pos() - get_pos()))
                        enemy = oo;
                }
            });
            if (enemy) {
                if (!try_see_at(enemy, enemy->get_pos())
                    && !try_see_at(enemy, enemy->get_pos() + Vec(0.4, 0.4))
                    && !try_see_at(enemy, enemy->get_pos() + Vec(-0.4, 0.5))
                    && !try_see_at(enemy, enemy->get_pos() + Vec(0, 1))
                    && !try_see_at(enemy, enemy->get_pos() + Vec(0, 1.4))
                ) {
                    enemy = NULL;
                }
            }
            if (enemy != old_enemy) {
                b2body->SetAwake(true);
                log("robot", "Enemy detected: %p", enemy);
            }
        }

        bool try_see_at (Object* obj, Vec o_pos) {
            if ((o_pos.x - get_pos().x) * direction < 0) return false;
            bool r = false;
            space.ray_cast(get_pos() + focus_offset(), o_pos, [&](b2Fixture* fix, const Vec& p, const Vec& n, float fraction){
                if ((Object*)fix->GetBody()->GetUserData() == obj) {
                    r = true;
                    return fraction;
                }
                else {
                    r = false;
                    return 0.f;
                }
            });
            if (r)
                enemy_pos = o_pos;
            return r;
        }
    };

    struct Patroller : Robot, phys::Grounded, Damagable {
        enum Fixture_Index {
            BODY,
            FLOOR_SENSOR_L,
            FLOOR_SENSOR_R
        };
        enum Frame_Index {
            UP,
            DOWN
        };
        Vec target = Vec(0, 0);
        float stride_phase = 0;  // Stored
        float oldxrel = 0;  // Set per frame
        uint32 attack_timer = 0;

        void Object_before_move () override {
            Robot::Object_before_move();
            auto def = get_def();
            if (ground) {
                 // Turn around at end of platform
                auto floor_sensor = &def->fixtures[direction < 0 ? FLOOR_SENSOR_L : FLOOR_SENSOR_R];
                bool sensed = false;
                foreach_contact([&](b2Fixture* mine, b2Fixture* other){
                    auto fd = (phys::FixtureDef*)mine->GetUserData();
                    if (fd == floor_sensor)
                        sensed = true;
                });
                if (controller) {
                    if (buttons & LEFT_BIT && !(buttons & RIGHT_BIT)) direction = -1;
                    if (buttons & RIGHT_BIT && !(buttons & LEFT_BIT)) direction = 1;
                }
                else {
                    if (!sensed) direction = -direction;
                }
                oldxrel = get_pos().x - ground->get_pos().x;
            }
            if (controller) {
                if (attack_timer) --attack_timer;
                else if (buttons & ATTACK_BIT) {
                    Vec bullet_pos = get_pos() + focus_offset();
                    Vec bullet_vel = 2 * normalize(focus);
                    log("robot", "shooting [%f %f] [%f %f]", bullet_pos.x, bullet_pos.y, bullet_vel.x, bullet_vel.y);
                    shoot_bullet(this, get_pos() + focus_offset(), bullet_vel);
                    attack_timer = 60;
                }
            }
            else {
                if (attack_timer) {
                    focus = 0.8*focus + 0.2*(target - get_pos() - focus_offset());
                    if (--attack_timer == 0) {
                        Vec bullet_pos = get_pos() + focus_offset();
                        Vec bullet_vel = 2 * normalize(focus);
                        log("robot", "shooting [%f %f] [%f %f]", bullet_pos.x, bullet_pos.y, bullet_vel.x, bullet_vel.y);
                        shoot_bullet(this, get_pos() + focus_offset(), bullet_vel);
                    }
                }
                if (enemy && !attack_timer) {
                    target = enemy_pos;
                    attack_timer = 60;
                }
            }
        }
        void Object_after_move () override {
            if (ground) {
                stride_phase += fabs(get_pos().x - ground->get_pos().x - oldxrel) / 0.7;
                stride_phase = fmod(stride_phase, 1);
            }
            else {
                stride_phase = 0;
            }
            if (enemy) {
                if (auto biped = dynamic_cast<Biped*>(enemy)) {
                    biped->vision.attend(get_pos() + Rect(-0.5, 0, 0.5, 1), 10);
                    vision.attend(biped->get_pos() + Rect(-0.5, 0, 0.5, 2), 10);
                }
            }
            Vec origin = get_pos() + focus_offset();
            vision.attend(origin + Rect(-1, -1, 1, 1), 1000000);
            Vec focus_world = focus + origin;
            vision_pos = vision.look(origin, &focus_world, !!controller);
            focus = focus_world - origin;
        }
        float Grounded_velocity () override {
            if (controller)
                return buttons & LEFT_BIT ? -2.0 : buttons & RIGHT_BIT ? 2.0 : 0.0;
            else
                return enemy ? 0.0 : 2.0 * direction;
        }
        void Drawn_draw (Sprites) override {
            auto def = get_def();
            float dir = get_vel().x > 0 ? 1 : get_vel().x < 0 ? -1 : direction;
            auto frame_i = stride_phase < 0.5 ? UP : DOWN;
            draw_frame(&def->layout->frames[frame_i], def->texture, get_pos(), Vec(dir, 1));
        }
        int32 life = 96;
        void Damagable_damage (int32 d) override {
            life -= d;
            if (life <= 0) {
                state_document()->destroy(this);
            }
        }
        Patroller () { direction = -1; }
    };

} using namespace ent;

HACCABLE(Ext_Def) {
    name("ent::Ext_Def");
    attr("Object_Def", base<Object_Def>().collapse());
    attr("texture", member(&Ext_Def::texture));
    attr("layout", member(&Ext_Def::layout));
}

HACCABLE(Robot) {
    name("ent::Robot");
    attr("ROD", base<ROD<Sprites, Ext_Def>>().collapse());
    attr("Controllable", base<Controllable>().collapse());
    attr("direction", member(&Patroller::direction).optional());
}

HACCABLE(Patroller) {
    name("ent::Patroller");
    attr("Robot", base<Robot>().collapse());
    attr("stride_phase", member(&Patroller::stride_phase).optional());
    attr("life", member(&Patroller::life).optional());
}

