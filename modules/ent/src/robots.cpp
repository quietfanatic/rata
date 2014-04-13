#include "ent/inc/bipeds.h"
#include "ent/inc/bullets.h"
#include "ent/inc/control.h"
#include "ent/inc/mixins.h"
#include "geo/inc/ground.h"
#include "geo/inc/vision.h"
#include "hacc/inc/everything.h"
#include "shell/inc/main.h"
#include "util/inc/debug.h"
#include "util/inc/geometry.h"
#include "vis/inc/images.h"
using namespace geo;
using namespace util;
using namespace vis;

namespace ent {

    struct Ext_Def : Agent_Def {
        vis::Texture* texture;
        vis::Layout* layout;
    };

     // Robots cannot see through walls.
    struct Robot : Agent<Sprites, Ext_Def> {
         // This is not stored.  TODO: should it be?
        Object* enemy = NULL;
        Vec enemy_pos = Vec(0, 0);
        int8 direction = 0;

         // All this does is do some ray casts to make sure we have a direct
         //  line of sight to the human.  TODO: check Filters instead of dynamic cast to Biped
        virtual void Object_before_move () override {
            auto old_enemy = enemy;
            enemy = NULL;
            b2PolygonShape shape;
            shape.SetAsBox(direction == 0 ? 20 : 10, 15);
            Vec query_pos = get_pos() + direction * Vec(10, 0);
            space.query_shape(query_pos, &shape, [&](b2Fixture* fix){
                auto oo = (Object*)fix->GetBody()->GetUserData();
                if (!enemy || length2(oo->get_pos() - get_pos()) < length2(enemy->get_pos() - get_pos()))
                    enemy = oo;
                return true;
            }, Filter(0x0004, 0x0002));
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
            auto def = get_def();
            bool r = false;
            space.ray_cast(get_pos() + def->focus_offset, o_pos, [&](b2Fixture* fix, const Vec& p, const Vec& n, float fraction){
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

    struct Patroller : Robot, Grounded {
        enum Fixture_Index {
            BODY,
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
                if (controller) {
                    if (buttons & LEFT_BIT && !(buttons & RIGHT_BIT)) direction = -1;
                    if (buttons & RIGHT_BIT && !(buttons & LEFT_BIT)) direction = 1;
                }
                else {
                    Vec sensor_pos = get_pos() + Vec(direction * 0.75, -0.1);
                    b2PolygonShape sensor_shape;
                    sensor_shape.SetAsBox(0.2, 0.2);
                    bool floor = space.query_shape(sensor_pos, &sensor_shape, nullptr, Filter(0x0001, 0x0001));
                    sensor_pos = get_pos() + Vec(direction * 0.75, 0.5);
                    sensor_shape.SetAsBox(0.2, 0.2);
                    bool wall = space.query_shape(sensor_pos, &sensor_shape, nullptr, Filter(0x0001, 0x0001));
                    if (wall || !floor) direction = -direction;
                }
                oldxrel = get_pos().x - ground->get_pos().x;
            }
            if (controller) {
                if (attack_timer) --attack_timer;
                else if (buttons & ATTACK_BIT) {
                    Vec bullet_pos = get_pos() + def->focus_offset;
                    Vec bullet_vel = 2 * normalize(focus);
                    log("robot", "shooting [%f %f] [%f %f]", bullet_pos.x, bullet_pos.y, bullet_vel.x, bullet_vel.y);
                    shoot_bullet(this, get_pos() + def->focus_offset, bullet_vel);
                    attack_timer = 60;
                }
            }
            else {
                if (attack_timer) {
                    focus = 0.8*focus + 0.2*(target - get_pos() - def->focus_offset);
                    if (--attack_timer == 0) {
                        Vec bullet_pos = get_pos() + def->focus_offset;
                        Vec bullet_vel = 2 * normalize(focus);
                        log("robot", "shooting [%f %f] [%f %f]", bullet_pos.x, bullet_pos.y, bullet_vel.x, bullet_vel.y);
                        shoot_bullet(this, get_pos() + def->focus_offset, bullet_vel);
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
//                    biped->vision.attend(get_pos() + Rect(-0.5, 0, 0.5, 1), 10);
                    vision.attend(biped->get_pos() + Rect(-0.5, 0, 0.5, 2), 10);
                }
            }
            Robot::Object_after_move();
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
        void Damagable_damage (int32 d) override {
            Agent::Damagable_damage(d);
            if (life <= 0) {
                state_document()->destroy(this);
            }
        }
        Patroller () { direction = -1; }
    };


    struct Flyer : Robot {
        enum Fixture_Index {
            BODY,
        };
        Vec target = Vec(NAN, NAN);
        uint32 attack_timer = 0;
        uint32 stun_timer = 0;
        uint32 frame_timer = 0;

        void Drawn_draw (Sprites) override {
            auto def = get_def();
            float down_angle = angle_diff(angle(focus), -M_PI / 2);
            auto frame_i = lround(down_angle / M_PI * 8);
            if (frame_timer >= 2) frame_i += 9;
            Vec scale = focus.x > 0 ? Vec(-1, 1) : Vec(1, 1);
            draw_frame(&def->layout->frames[frame_i], def->texture, get_pos(), scale);
        }
        void Damagable_damage (int32 d) override {
            Agent::Damagable_damage(d);
            if (life <= 0) {
                state_document()->destroy(this);
            }
        }

        void Object_before_move () override {
            Robot::Object_before_move();
            auto def = get_def();
             // Attack logic
            if (controller) {
                Vec acc = Vec(0, 0);
                if (buttons & LEFT_BIT)
                    acc.x -= 0.2;
                if (buttons & DOWN_BIT)
                    acc.y -= 0.2;
                if (buttons & RIGHT_BIT)
                    acc.x += 0.2;
                if (buttons & UP_BIT)
                    acc.y += 0.2;
                if (buttons) {
                    set_vel(get_vel() + acc);
                }
                if (attack_timer) --attack_timer;
                else if (buttons & ATTACK_BIT) {
                    Vec bullet_pos = get_pos() + def->focus_offset;
                    Vec bullet_vel = 2 * normalize(focus);
                    log("robot", "shooting [%f %f] [%f %f]", bullet_pos.x, bullet_pos.y, bullet_vel.x, bullet_vel.y);
                    shoot_bullet(this, get_pos() + def->focus_offset, bullet_vel);
                    attack_timer = 90;
                }
            }
            else {
                if (!defined(target)) target = get_pos();
                if (stun_timer) stun_timer--;
                if (attack_timer) {
                    focus = 0.8*focus + 0.2*(target - get_pos() - def->focus_offset);
                    if (--attack_timer == 0) {
                        Vec bullet_pos = get_pos() + def->focus_offset;
                        Vec bullet_vel = 2 * normalize(focus);
                        log("robot", "shooting [%f %f] [%f %f]", bullet_pos.x, bullet_pos.y, bullet_vel.x, bullet_vel.y);
                        shoot_bullet(this, get_pos() + def->focus_offset, bullet_vel);
                    }
                }
                else if (enemy) {
                    target = enemy_pos;
                    attack_timer = 90;
                }
                else if (!stun_timer && length2(target - get_pos()) > 0.15) {
                     // Chase the last seen enemy
                    float acc = 8.0;
                    Vec rel_target = target - get_pos();
                    float vel_to_target = dot(get_vel(), normalize(rel_target));
                    bool off_track = vel_to_target < length(get_vel()) - 1.0;
                     // We have: p, v, a. We need d.
                     // d = p + vt + att/2; since v = at, t = v/a
                     // d = p + vv/a + vv/a/2;
                     // d = p + 3vv/2a
                     // Weird, but that's what the math says.
                    float stop_distance = 1.5 * vel_to_target * vel_to_target / acc - 0.2;
                    Vec force = Vec(frand() * 4 - 2, frand() * 4 - 2);
                    if (off_track) {
                        if (get_vel() != Vec(0, 0)) {
                            force += acc * normalize(-get_vel());
                        }
                    }
                    else if (rel_target == Vec(0, 0)) {
                    }
                    else if (stop_distance >= length(rel_target)) {
                        force -= acc * normalize(rel_target);
                    }
                    else if (rel_target != Vec(0, 0)) {
                        force += acc * normalize(rel_target);
                    }
                    b2body->ApplyForceToCenter(force, true);
                }
            }
        }
        void Object_after_move () override {
            foreach_contact([&](b2Fixture* mine, b2Fixture* other){
                if (other->IsSensor()) return;
                auto fd = (FixtureDef*)mine->GetUserData();
                if (fd == &def->fixtures[BODY]) {
                    stun_timer = 30;
                }
            });
            if (enemy) {
                if (auto biped = dynamic_cast<Biped*>(enemy)) {
//                    biped->vision.attend(get_pos() + Rect(-0.5, 0, 0.5, 1), 10);
                    vision.attend(biped->get_pos() + Rect(-0.5, 0, 0.5, 2), 10);
                }
            }
            Robot::Object_after_move();
            frame_timer += 1;
            frame_timer %= 4;
        }
        Flyer () { focus = Vec(0, -1); }

        size_t Spatial_n_pts () override { return 1; }
        Vec Spatial_get_pt (size_t) override { return target - get_pos(); }
        void Spatial_set_pt (size_t, Vec p) override { target = p + get_pos(); }

    };

} using namespace ent;

HACCABLE(Ext_Def) {
    name("ent::Ext_Def");
    attr("Agent_Def", base<Agent_Def>().collapse());
    attr("texture", member(&Ext_Def::texture));
    attr("layout", member(&Ext_Def::layout));
}

HACCABLE(Robot) {
    name("ent::Robot");
    attr("Agent", base<Agent<Sprites, Ext_Def>>().collapse());
    attr("Controllable", base<Controllable>().collapse());
    attr("direction", member(&Patroller::direction).optional());
}

HACCABLE(Patroller) {
    name("ent::Patroller");
    attr("Robot", base<Robot>().collapse());
    attr("stride_phase", member(&Patroller::stride_phase).optional());
}

HACCABLE(Flyer) {
    name("ent::Flyer");
    attr("Robot", base<Robot>().collapse());
    attr("target", member(&Flyer::target).optional());
    attr("stun_timer", member(&Flyer::stun_timer).optional());
}

