#include "ent/inc/bipeds.h"
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
    struct Robot : ROD<Sprites, Ext_Def> {
         // This is not stored.  TODO: should it be?
        Object* enemy = NULL;
        Vec enemy_pos;
        int8 direction = 0;

         // All this does is do some ray casts to make sure we have a direct
         //  line of sight to the human.  TODO: check Filters instead of dynamic cast to Biped
        virtual void Object_before_move () override {
            auto old_enemy = enemy;
            enemy = NULL;
            foreach_contact([&](b2Fixture* mine, b2Fixture* other){
                auto fd = (phys::FixtureDef*)mine->GetUserData();
                auto oo = (Object*)other->GetBody()->GetUserData();
                if (dynamic_cast<Biped*>(oo)) {
                    if (!enemy || length2(oo->pos() - pos()) < length2(enemy->pos() - pos()))
                        enemy = oo;
                }
            });
            if (enemy) {
                if (!try_see_at(enemy, enemy->pos())
                        && !try_see_at(enemy, enemy->pos() + Vec(0.4, 0.4))
                        && !try_see_at(enemy, enemy->pos() + Vec(-0.4, 0.5))
                        && !try_see_at(enemy, enemy->pos() + Vec(0, 1))
                        && !try_see_at(enemy, enemy->pos() + Vec(0, 1.4)))
                    enemy = NULL;
            }
            if (enemy != old_enemy) {
                b2body->SetAwake(true);
                log("robot", "Enemy detected: %p", enemy);
            }
        }

        bool try_see_at (Object* obj, Vec o_pos) {
            if ((o_pos.x - pos().x) * direction < 0) return false;
            bool r = false;
            space.ray_cast(pos() + Vec(0, 0.5), o_pos, [&](b2Fixture* fix, const Vec& p, const Vec& n, float fraction){
                if ((Object*)fix->GetBody()->GetUserData() == obj) {
                    r = true;
                    return fraction;
                }
                else {
                    r = false;
                    return 0.f;
                }
            });
            if (r) enemy_pos = o_pos;
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
        float stride_phase = 0;  // Stored
        float oldxrel = 0;  // Set per frame

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
                if (!sensed) direction = -direction;
                oldxrel = pos().x - ground->pos().x;
            }
        }
        void Object_after_move () override {
            if (ground) {
                stride_phase += fabs(pos().x - ground->pos().x - oldxrel) / 0.7;
                stride_phase = fmod(stride_phase, 1);
            }
            else {
                stride_phase = 0;
            }
            if (enemy) {
                if (auto biped = dynamic_cast<Biped*>(enemy)) {
                    biped->vision.attend(pos() + Rect(-0.5, 0, 0.5, 1), 10);
                }
            }
        }
        float Grounded_velocity () override {
            return enemy ? 0.0 : 2.0 * direction;
        }
        void Drawn_draw (Sprites) override {
            auto def = get_def();
            float dir = vel().x > 0 ? 1 : vel().x < 0 ? -1 : direction;
            auto frame_i = stride_phase < 0.5 ? UP : DOWN;
            draw_frame(&def->layout->frames[frame_i], def->texture, pos(), Vec(dir, 1));
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
    attr("direction", member(&Patroller::direction).optional());
}

HACCABLE(Patroller) {
    name("ent::Patroller");
    attr("Robot", base<Robot>().collapse());
    attr("stride_phase", member(&Patroller::stride_phase).optional());
    attr("life", member(&Patroller::life).optional());
}

