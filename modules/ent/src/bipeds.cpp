
#include "../inc/bipeds.h"
#include "../../hacc/inc/everything.h"
#include "../../util/inc/debug.h"

namespace ent {

     // Definitions
    void Biped::set_def (BipedDef* _def) {
        def = _def;
        apply_bdf(def->body_def);
        model.apply_skel(def->skel);
        stats = *def->stats;
    }

    Vec Biped::Resident_pos () {
        return pos();
    }
    void Biped::Resident_emerge () {
        materialize();
        appear();
    }
    void Biped::Resident_reclude () {
        dematerialize();
        disappear();
    }

    void Biped::Controllable_buttons (ButtonBits bits) {
        buttons = bits;
    }
    void Biped::Controllable_move_focus (Vec diff) {
        focus = constrain(focus + diff, Rect(-18, -13, 18, 13));
    }
    Vec Biped::Controllable_get_focus () {
        return focus + pos();
    }

    void Biped::before_move () {
        int8 mdir = move_direction();
         // Change some kinds of movement state
         // Do not change ground velocity, but do change air velocity
        if (ground) {
             // Turn around
            if (!crawling || !ceiling_low) {
                switch (mdir) {
                    case -1: {
                        if (vel().x <= 0)
                            direction = -1;
                        break;
                    }
                    case 1: {
                        if (vel().x >= 0)
                            direction = 1;
                        break;
                    }
                    default: break;
                }
            }
             // Decide whether we're crouching
            if (buttons & CROUCH_BIT) {
                crouching = true;
                if (mdir)
                    crawling = true;
            }
            else if (!crouching || !ceiling_low) {
                crouching = false;
                crawling = false;
            }
             // For walking animation
            oldxrel = pos().x - ground->pos().x;
             // Initiate jump
            if (buttons & JUMP_BIT) {
                 // TODO: jump delay
                set_vel(Vec(vel().x, stats.jump_speed));
            }
        }
        else {  // In the air
            if (vel().x * mdir <= stats.air_speed - stats.air_friction) {
                set_vel(Vec(vel().x + stats.air_friction * mdir, vel().y));
            }
            else if (vel().x * mdir <= stats.air_speed) {
                set_vel(Vec(stats.air_speed * mdir, vel().y));
            }
        }
         // Change active fixture
        auto active = (
            ground
                ? crawling
                    ? mdir == 1
                        ? &def->fixdefs->crawl_r
                        : &def->fixdefs->crawl_l
                    : crouching
                        ? &def->fixdefs->crouch
                    : &def->fixdefs->stand
                : &def->fixdefs->stand
        );
        for (auto fix = b2body->GetFixtureList(); fix; fix = fix->GetNext()) {
            auto fd = (phys::FixtureDef*)fix->GetUserData();
//            if (def->fixdefs->is_primary(fd)) {
                fix->SetSensor(fd != active);
//            }
        }
    }
    float Biped::Grounded_velocity () {
        if (crouching)
            return stats.crawl_speed * move_direction();
        else 
            return stats.run_speed * move_direction();
    }
    float Biped::Grounded_friction () {
        if (crouching) {
            return stats.crawl_friction;
        }
        else if (int8 dir = move_direction()) {
            if (vel().x * dir >= stats.walk_speed)
                return stats.run_friction;
            else if (vel().x * dir >= 0)
                return stats.walk_friction;
            else
                return stats.skid_friction;
        }
        else return stats.stop_friction;
    }
     // Ensure we're in the right room and also calculate walk animation.
    void Biped::after_move () {
         // Clear input before next frame
        buttons = ButtonBits(0);
        reroom();
        if (ground && (!crouching || crawling)) {
            if (fabs(vel().x) < 0.01) {
                distance_walked = 0;
            }
            else {
                distance_walked += fabs(pos().x - ground->pos().x - oldxrel);
            }
        }
        else {
            distance_walked = 0;
        }
         // Read sensors
        ceiling_low = false;
        foreach_contact([&](b2Fixture* mine, b2Fixture* other){
            auto fd = (phys::FixtureDef*)mine->GetUserData();
            if (fd == &def->fixdefs->ceiling_low) {
                ceiling_low = true;
            }
        });
    }

    void Biped::Sprite_draw () {
        model.apply_skin(def->skin);
        if (ground) {
            if (crawling) {
                if (fabs(vel().x) < 0.01) {
                    model.apply_pose(&def->poses->crawl1);
                }
                else {
                    float stepdist = fmod(distance_walked, 2.0);
                    if (stepdist < 0.5)
                        model.apply_pose(&def->poses->crawl2_1);
                    else if (stepdist >= 1 && stepdist < 1.5)
                        model.apply_pose(&def->poses->crawl2_2);
                    else
                        model.apply_pose(&def->poses->crawl1);
                }
            }
            else if (crouching) {
                model.apply_pose(&def->poses->crouch);
            }
            else {
                if (fabs(vel().x) < 0.01) {
                    model.apply_pose(&def->poses->stand);
                }
                else {
                    float stepdist = fmod(distance_walked, 2.0);
                    if (stepdist < 0.5)
                        model.apply_pose(&def->poses->walk1);
                    else if (stepdist >= 1 && stepdist < 1.5)
                        model.apply_pose(&def->poses->walk2);
                    else
                        model.apply_pose(&def->poses->stand);
                }
            }
        }
        else {
            model.apply_pose(&def->poses->walk1);
        }
        model.draw(pos(), direction < 0);
    }

     // Kinda weird that neither of these has anything
    Biped::Biped () { }
    void Biped::finish () { }

} using namespace ent;

HCB_BEGIN(Biped)
    name("ent::Biped");
    attr("def", value_methods(&Biped::get_def, &Biped::set_def));
    attr("Object", base<phys::Object>());
    attr("Resident", base<geo::Resident>());
    attr("Grounded", base<phys::Grounded>().optional());
    attr("Controllable", base<ent::Controllable>().optional());
    attr("direction", member(&Biped::direction).optional());
    attr("distance_walked", member(&Biped::distance_walked).optional());
    attr("crouching", member(&Biped::crouching).optional());
    attr("crawling", member(&Biped::crawling).optional());
    attr("ceiling_low", member(&Biped::ceiling_low).optional());
    attr("focus", member(&Biped::focus).optional());
    finish([](Biped& b){
        b.Resident::finish();
        b.finish();
    });
HCB_END(Biped)

HCB_BEGIN(BipedStats)
    name("ent::BipedStats");
    attr("walk_friction", member(&BipedStats::walk_friction).optional());
    attr("walk_speed", member(&BipedStats::walk_speed).optional());
    attr("run_friction", member(&BipedStats::run_friction).optional());
    attr("run_speed", member(&BipedStats::run_speed).optional());
    attr("crawl_friction", member(&BipedStats::crawl_friction).optional());
    attr("crawl_speed", member(&BipedStats::crawl_speed).optional());
    attr("stop_friction", member(&BipedStats::stop_friction).optional());
    attr("skid_friction", member(&BipedStats::skid_friction).optional());
    attr("air_friction", member(&BipedStats::air_friction).optional());
    attr("air_speed", member(&BipedStats::air_speed).optional());
    attr("jump_speed", member(&BipedStats::jump_speed).optional());
HCB_END(BipedStats)

HCB_BEGIN(BipedDef)
    name("ent::BipedDef");
    attr("body_def", member(&BipedDef::body_def));
    attr("fixdefs", member(&BipedDef::fixdefs));
    attr("stats", member(&BipedDef::stats));
    attr("skel", member(&BipedDef::skel));
    attr("poses", member(&BipedDef::poses));
    attr("skin", member(&BipedDef::skin));
HCB_END(BipedDef)

HCB_BEGIN(BipedPoses)
    name("ent::BipedPoses");
    attr("stand", member(&BipedPoses::stand));
    attr("walk1", member(&BipedPoses::walk1));
    attr("walk2", member(&BipedPoses::walk2));
    attr("crouch", member(&BipedPoses::crouch));
    attr("crawl1", member(&BipedPoses::crawl1));
    attr("crawl2_1", member(&BipedPoses::crawl2_1));
    attr("crawl2_2", member(&BipedPoses::crawl2_2));
    attr("hurtbk", member(&BipedPoses::hurtbk));
    attr("laybk", member(&BipedPoses::laybk));
    attr("look_stand", member(&BipedPoses::look_stand));
    attr("look_walk", member(&BipedPoses::look_walk));
HCB_END(BipedPoses)

HCB_BEGIN(BipedFixdefs)
    name("ent::BipedFixdefs");
    attr("stand", member(&BipedFixdefs::stand));
    attr("hurt", member(&BipedFixdefs::hurt));
    attr("crouch", member(&BipedFixdefs::crouch));
    attr("crawl_l", member(&BipedFixdefs::crawl_l));
    attr("crawl_r", member(&BipedFixdefs::crawl_r));
    attr("ceiling_low", member(&BipedFixdefs::ceiling_low));
HCB_END(BipedFixdefs)
