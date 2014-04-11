
#include <vector>
#include "ent/inc/bipeds.h"
#include "ent/inc/mixins.h"
#include "hacc/inc/everything.h"
#include "phys/inc/ground.h"
#include "shell/inc/main.h"
#include "util/inc/geometry.h"
#include "vis/inc/images.h"
#include "vis/inc/light.h"

using namespace util;
using namespace vis;
using namespace phys;

namespace ent {

    struct Inert_Def : Object_Def {
        Texture* texture = NULL;
        Rect boundary;
        int32 life = -1;  // If negative, invincible
    };

    struct Ext_Def : Object_Def {
        vis::Texture* texture;
        vis::Layout* layout;
    };

    struct Inert : ROD<Sprites, Inert_Def>, Damagable {
        int32 life = -1;
        void finish () {
            if (life < 0) life = get_def()->life;
            ROD::finish();
        }
        void Drawn_draw (Sprites) override {
            auto def = get_def();
            draw_texture(def->texture, pos() + def->boundary);
        }
        void Damagable_damage (int32 d) override {
            if (life >= 0) {
                life -= d;
                if (life <= 0) {
                    state_document()->destroy(this);
                }
            }
        }
    };

    struct Light : geo::Resident, Drawn<Lights> {
        Vec pos;
        std::vector<Vec> vertices;
        RGBf ambient;
        RGBf diffuse;
        RGBf radiant;
        void Drawn_draw (Lights) override {
            light_offset(pos);
            set_ambient(ambient);
            set_diffuse(diffuse);
            set_radiant(radiant);
            draw_light(vertices.size(), vertices.data());
        }
        Vec Resident_get_pos () override { return pos; }
        void Resident_set_pos (Vec p) override { pos = p; }
        void Resident_emerge () override { appear(); }
        void Resident_reclude () override { disappear(); }
        size_t Resident_n_pts () override { return vertices.size(); }
        Vec Resident_get_pt (size_t i) override { return vertices.at(i); }
        void Resident_set_pt (size_t i, Vec v) override {
            vertices.at(i) = v;
        }
    };

     // Robots cannot see through walls.
    struct Robot : ROD<Sprites, Ext_Def> {
         // This is not stored.  TODO: should it be?
        Object* enemy = NULL;
        Vec enemy_pos;

         // All this does is do some ray casts to make sure we have a direct
         //  line of sight to the human.  TODO: check Filters instead of dynamic cast to Biped
        virtual void Object_before_move () override {
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
        }

        bool try_see_at (Object* obj, Vec o_pos) {
            bool r = false;
            space.ray_cast(ROD::pos(), o_pos, [&](b2Fixture* fix, const Vec& p, const Vec& n, float fraction){
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
        int8 direction = -1;
        float stride_phase = 0;  // Stored
        float oldxrel = 0;  // Set per frame

        void Object_before_move () override {
            auto def = get_def();
            if (ground) {
                Robot::Object_before_move();
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
        }
        float Grounded_velocity () override {
            return enemy ? 0 : 2 * direction;
        }
        void Drawn_draw (Sprites) override {
            auto def = get_def();
            float dir = vel().x >= 0 ? 1 : -1;
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
    };

} using namespace ent;

HACCABLE(Inert_Def) {
    name("ent::Inert_Def");
    attr("Object_Def", base<Object_Def>().collapse());
    attr("texture", member(&Inert_Def::texture));
    attr("boundary", member(&Inert_Def::boundary));
    attr("life", member(&Inert_Def::life).optional());
}

HACCABLE(Ext_Def) {
    name("ent::Ext_Def");
    attr("Object_Def", base<Object_Def>().collapse());
    attr("texture", member(&Ext_Def::texture));
    attr("layout", member(&Ext_Def::layout));
}

HACCABLE(Inert) {
    name("ent::Inert");
    attr("ROD", base<ROD<Sprites, Inert_Def>>().collapse());
    attr("life", member(&Inert::life).optional());
    finish(&Inert::finish);
}

HACCABLE(Light) {
    name("ent::Light");
    attr("Resident", base<geo::Resident>().collapse());
    attr("pos", member(&Light::pos).optional());
    attr("vertices", member(&Light::vertices).optional());
    attr("ambient", member(&Light::ambient).optional());
    attr("diffuse", member(&Light::diffuse).optional());
    attr("radiant", member(&Light::radiant).optional());
}

HACCABLE(Robot) {
    name("ent::Robot");
    attr("ROD", base<ROD<Sprites, Ext_Def>>().collapse());
}

HACCABLE(Patroller) {
    name("ent::Patroller");
    attr("Robot", base<Robot>().collapse());
    attr("direction", member(&Patroller::direction).optional());
    attr("stride_phase", member(&Patroller::stride_phase).optional());
    attr("life", member(&Patroller::life).optional());
}

