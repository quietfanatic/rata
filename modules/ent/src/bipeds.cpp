
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
         // Since we walk by altering our contact with the ground,
         //  and since contacts aren't processed if the body is asleep,
         //  then if the body falls asleep we can't move T_T
        if (bits)
            b2body->SetAwake(true);
        buttons = bits;
    }
    void Biped::Controllable_move_focus (Vec diff) {
        focus = constrain(focus + diff, Rect(-18, -13, 18, 13));
    }
    Vec Biped::Controllable_get_focus () {
        return focus + pos() + def->focus_offset;
    }

     // Change some kinds of movement state
     // Do not change ground velocity, but do change air velocity
    void Biped::before_move () {
        int8 mdir = move_direction();
         // Turn around
        if (!crawling || !ceiling_low) {
            direction = focus.x > 0 ? 1 : focus.x < 0 ? -1 : direction;
        }
        if (ground) {
            if (ceiling_low) {
                crouching = true;
                crawling = true;
            }
            else if (jump_timer) {
                if (jump_timer >= stats.jump_delay / FR) {
                    set_vel(Vec(vel().x, stats.jump_speed));
                    jump_timer = 0;
                    ground = NULL;
                }
                else if (buttons & JUMP_BIT) {
                    jump_timer++;
                }
                else {  // Short jump
                    set_vel(Vec(vel().x, stats.jump_speed * 2 / 3));
                    jump_timer = 0;
                    ground = NULL;
                }
            }
            else if (buttons & CROUCH_BIT) {
                crouching = true;
                if (mdir)
                    crawling = true;
            }
            else if (!crouching || !ceiling_low) {
                crouching = false;
                crawling = false;
                 // Initiate jump
                if (buttons & JUMP_BIT) {
                    jump_timer++;
                }
            }
        }
        else {  // In the air
            crouching = false;
            crawling = false;
            jump_timer = 0;
            if (vel().x * mdir <= stats.air_speed - stats.air_friction) {
                set_vel(Vec(vel().x + stats.air_friction * mdir, vel().y));
            }
            else if (vel().x * mdir <= stats.air_speed) {
                set_vel(Vec(stats.air_speed * mdir, vel().y));
            }
        }
         // For walking animation
        if (ground)
            oldxrel = pos().x - ground->pos().x;
        else
            oldxrel = 0;
         // Change active fixture
        auto active = (
            ground
                ? jump_timer
                    ? &def->fixdefs->crouch
                    : crawling
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
            if (def->fixdefs->is_primary(fd)) {
                phys::Filter filt = fix->GetFilterData();
                if (filt.active != (fd == active)) {
                    filt.active = (fd == active);
                    fix->SetFilterData(filt);
                }
            }
        }
    }
    float Biped::Grounded_velocity () {
        int8 mdir = move_direction();
        if (crouching)
            return stats.crawl_speed * mdir;
        else if (direction == mdir)
            return stats.run_speed * mdir;
        else 
            return stats.walk_speed * mdir;
    }
    float Biped::Grounded_friction () {
        if (crouching) {
            return stats.crawl_friction;
        }
        else if (int8 mdir = move_direction()) {
            if (vel().x * mdir > stats.walk_speed)
                return stats.run_friction;
            else if (vel().x * mdir >= 0)
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

    void Biped::Sprites_draw (vis::Sprites_Renderer r) {
        model.apply_skin(def->skin);
        uint8 look_frame = angle_frame(atan2(focus.y, focus.x));
        if (ground) {
            if (jump_timer) {
                model.apply_pose(&def->poses->prejump);
                model.apply_pose(&def->poses->look_stand[look_frame]);
            }
            else if (crawling) {
                if (fabs(vel().x) < 0.01) {
                    model.apply_pose(&def->poses->crawl[3]);
                }
                else {
                    uint step = fmod(distance_walked * 2.0, 4.0);
                    model.apply_pose(&def->poses->crawl[step]);
                }
            }
            else if (crouching) {
                model.apply_pose(&def->poses->crouch);
                model.apply_pose(&def->poses->look_stand[look_frame]);
            }
            else if (vel().x * direction > stats.walk_speed) {
                float stepdist = fmod(distance_walked * 1.5, 6.0);
                 // Expand frames 1 and 4 a little
                uint step =
                    stepdist < 0.9 ? 0
                  : stepdist < 2.1 ? 1
                  : stepdist < 3   ? 2
                  : stepdist < 3.9 ? 3
                  : stepdist < 5.1 ? 4
                  :                  5;
                model.apply_pose(&def->poses->run[step]);
                if (step % 3 < 1)
                    model.apply_pose(&def->poses->look_stand[look_frame]);
                else
                    model.apply_pose(&def->poses->look_walk[look_frame]);
            }
            else if (fabs(vel().x) >= 0.01) {
                uint step = fmod(distance_walked * 2.0, 4.0);
                model.apply_pose(&def->poses->walk[step]);
                if (step % 2 < 1)
                    model.apply_pose(&def->poses->look_walk[look_frame]);
                else
                    model.apply_pose(&def->poses->look_stand[look_frame]);
            }
            else {
                model.apply_pose(&def->poses->stand);
                model.apply_pose(&def->poses->look_stand[look_frame]);
            }
        }
        else {
            model.apply_pose(&def->poses->jump);
            model.apply_pose(&def->poses->look_walk[look_frame]);
        }
        model.draw(r, pos(), direction < 0);
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
    attr("crouching", member(&Biped::crouching).optional());
    attr("crawling", member(&Biped::crawling).optional());
    attr("ceiling_low", member(&Biped::ceiling_low).optional());
    attr("jump_timer", member(&Biped::jump_timer).optional());
    attr("focus", member(&Biped::focus).optional());
    attr("distance_walked", member(&Biped::distance_walked).optional());
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
    attr("jump_delay", member(&BipedStats::jump_delay).optional());
HCB_END(BipedStats)

HCB_BEGIN(BipedDef)
    name("ent::BipedDef");
    attr("body_def", member(&BipedDef::body_def));
    attr("fixdefs", member(&BipedDef::fixdefs));
    attr("stats", member(&BipedDef::stats));
    attr("skel", member(&BipedDef::skel));
    attr("poses", member(&BipedDef::poses));
    attr("skin", member(&BipedDef::skin));
    attr("focus_offset", member(&BipedDef::focus_offset));
HCB_END(BipedDef)

HCB_BEGIN(BipedPoses)
    name("ent::BipedPoses");
    attr("stand", member(&BipedPoses::stand));
    attr("walk", member(&BipedPoses::walk));
    attr("run", member(&BipedPoses::run));
    attr("crouch", member(&BipedPoses::crouch));
    attr("prejump", member(&BipedPoses::prejump));
    attr("jump", member(&BipedPoses::jump));
    attr("crawl", member(&BipedPoses::crawl));
    attr("hurtbk", member(&BipedPoses::hurtbk));
    attr("laybk", member(&BipedPoses::laybk));
    attr("look_stand", member(&BipedPoses::look_stand));
    attr("look_walk", member(&BipedPoses::look_walk));
HCB_END(BipedPoses)

HCB_BEGIN(BipedFixdefs)
    name("ent::BipedFixdefs");
    attr("feet", member(&BipedFixdefs::feet));
    attr("stand", member(&BipedFixdefs::stand));
    attr("hurt", member(&BipedFixdefs::hurt));
    attr("crouch", member(&BipedFixdefs::crouch));
    attr("crawl_l", member(&BipedFixdefs::crawl_l));
    attr("crawl_r", member(&BipedFixdefs::crawl_r));
    attr("ceiling_low", member(&BipedFixdefs::ceiling_low));
HCB_END(BipedFixdefs)
