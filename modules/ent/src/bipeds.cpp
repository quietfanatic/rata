
#include "../inc/bipeds.h"
#include "../../geo/inc/camera.h"
#include "../../hacc/inc/everything.h"
#include "../../util/inc/debug.h"

namespace ent {

    void Biped::finish () {
        ROD::finish();
        auto def = static_cast<Biped_Def*>(Object::def);
        stats = *def->stats;
        for (auto fp = &def->fixdefs->feet; fp <= &def->fixdefs->ceiling_low; fp++) {
            add_fixture(fp);
        }
    }

    void Biped::Controllable_buttons (Button_Bits bits) {
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
        return focus + pos() + bpdef()->focus_offset;
    }

     // Change some kinds of movement state
     // Do not change ground velocity, but do change air velocity
    void Biped::Object_before_move () {
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
                    ? &bpdef()->fixdefs->crouch
                    : crawling
                        ? mdir == 1
                            ? &bpdef()->fixdefs->crawl_r
                            : &bpdef()->fixdefs->crawl_l
                        : crouching
                            ? &bpdef()->fixdefs->crouch
                        : &bpdef()->fixdefs->stand
                : &bpdef()->fixdefs->stand
        );
        for (auto fix = b2body->GetFixtureList(); fix; fix = fix->GetNext()) {
            auto fd = (phys::FixtureDef*)fix->GetUserData();
            if (bpdef()->fixdefs->is_primary(fd)) {
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
    void Biped::Object_after_move () {
         // Clear input before next frame
        buttons = Button_Bits(0);
        reroom(pos());
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
            if (fd == &bpdef()->fixdefs->ceiling_low) {
                ceiling_low = true;
            }
        });
         // Camera control
         // TODO: This kinda belongs somewhere else maybe?
         //  Well, it's gonna be replaced by a conspicuousity system anyway.
        if (controller) {
            geo::default_camera().pos = pos() + bpdef()->focus_offset
                                      + Vec(floor(focus.x/2/PX), floor(focus.y/2/PX))*PX;
        }
    }

    void Biped::Drawn_draw (vis::Sprites) {
        char model_data [bpdef()->skel->model_data_size()];
        vis::Model model (bpdef()->skel, model_data);
        uint8 look_frame = angle_frame(atan2(focus.y, focus.x));
        if (ground) {
            if (jump_timer) {
                model.apply_pose(&bpdef()->poses->prejump);
                model.apply_pose(&bpdef()->poses->look_stand[look_frame]);
            }
            else if (crawling) {
                if (fabs(vel().x) < 0.01) {
                    model.apply_pose(&bpdef()->poses->crawl[3]);
                }
                else {
                    uint step = fmod(distance_walked * 2.0, 4.0);
                    model.apply_pose(&bpdef()->poses->crawl[step]);
                }
            }
            else if (crouching) {
                model.apply_pose(&bpdef()->poses->crouch);
                model.apply_pose(&bpdef()->poses->look_stand[look_frame]);
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
                model.apply_pose(&bpdef()->poses->run[step]);
                if (step % 3 < 1)
                    model.apply_pose(&bpdef()->poses->look_stand[look_frame]);
                else
                    model.apply_pose(&bpdef()->poses->look_walk[look_frame]);
            }
            else if (fabs(vel().x) >= 0.01) {
                uint step = fmod(distance_walked * 2.0, 4.0);
                model.apply_pose(&bpdef()->poses->walk[step]);
                if (step % 2 < 1)
                    model.apply_pose(&bpdef()->poses->look_walk[look_frame]);
                else
                    model.apply_pose(&bpdef()->poses->look_stand[look_frame]);
            }
            else {
                model.apply_pose(&bpdef()->poses->stand);
                model.apply_pose(&bpdef()->poses->look_stand[look_frame]);
            }
        }
        else {
            model.apply_pose(&bpdef()->poses->jump);
            model.apply_pose(&bpdef()->poses->look_walk[look_frame]);
        }
         // TODO: implement this as an item
        size_t n_skins = 1 + equipment.items.count();
        vis::Skin* skins [n_skins];
        skins[0] = bpdef()->skin;
        size_t i = 0;
        for (auto& e : equipment.items) {
            skins[++i] = e.def->skin;
        }
        model.draw(n_skins, skins, pos(), Vec(direction, 1));
    }

    Biped::Biped () { }

} using namespace ent;

HACCABLE(Biped) {
    name("ent::Biped");
    attr("ROD", base<ROD<vis::Sprites>>().collapse());
    attr("Grounded", base<phys::Grounded>().collapse());
    attr("Controllable", base<ent::Controllable>().collapse());
    attr("direction", member(&Biped::direction).optional());
    attr("crouching", member(&Biped::crouching).optional());
    attr("crawling", member(&Biped::crawling).optional());
    attr("ceiling_low", member(&Biped::ceiling_low).optional());
    attr("jump_timer", member(&Biped::jump_timer).optional());
    attr("focus", member(&Biped::focus).optional());
    attr("inventory", member(&Biped::inventory).optional());
    attr("equipment", member(&Biped::equipment).optional());
    attr("distance_walked", member(&Biped::distance_walked).optional());
    finish(&Biped::finish);
}

HACCABLE(Biped_Stats) {
    name("ent::Biped_Stats");
    attr("walk_friction", member(&Biped_Stats::walk_friction).optional());
    attr("walk_speed", member(&Biped_Stats::walk_speed).optional());
    attr("run_friction", member(&Biped_Stats::run_friction).optional());
    attr("run_speed", member(&Biped_Stats::run_speed).optional());
    attr("crawl_friction", member(&Biped_Stats::crawl_friction).optional());
    attr("crawl_speed", member(&Biped_Stats::crawl_speed).optional());
    attr("stop_friction", member(&Biped_Stats::stop_friction).optional());
    attr("skid_friction", member(&Biped_Stats::skid_friction).optional());
    attr("air_friction", member(&Biped_Stats::air_friction).optional());
    attr("air_speed", member(&Biped_Stats::air_speed).optional());
    attr("jump_speed", member(&Biped_Stats::jump_speed).optional());
    attr("jump_delay", member(&Biped_Stats::jump_delay).optional());
}

HACCABLE(Biped_Def) {
    name("ent::Biped_Def");
    attr("Object_def", base<phys::Object_Def>().collapse());
    attr("fixdefs", member(&Biped_Def::fixdefs));
    attr("stats", member(&Biped_Def::stats));
    attr("skel", member(&Biped_Def::skel));
    attr("poses", member(&Biped_Def::poses));
    attr("skin", member(&Biped_Def::skin));
    attr("focus_offset", member(&Biped_Def::focus_offset));
}

HACCABLE(Biped_Poses) {
    name("ent::Biped_Poses");
    attr("stand", member(&Biped_Poses::stand));
    attr("walk", member(&Biped_Poses::walk));
    attr("run", member(&Biped_Poses::run));
    attr("crouch", member(&Biped_Poses::crouch));
    attr("prejump", member(&Biped_Poses::prejump));
    attr("jump", member(&Biped_Poses::jump));
    attr("crawl", member(&Biped_Poses::crawl));
    attr("hurtbk", member(&Biped_Poses::hurtbk));
    attr("laybk", member(&Biped_Poses::laybk));
    attr("look_stand", member(&Biped_Poses::look_stand));
    attr("look_walk", member(&Biped_Poses::look_walk));
}

HACCABLE(Biped_Fixdefs) {
    name("ent::Biped_Fixdefs");
    attr("feet", member(&Biped_Fixdefs::feet));
    attr("stand", member(&Biped_Fixdefs::stand));
    attr("hurt", member(&Biped_Fixdefs::hurt));
    attr("crouch", member(&Biped_Fixdefs::crouch));
    attr("crawl_l", member(&Biped_Fixdefs::crawl_l));
    attr("crawl_r", member(&Biped_Fixdefs::crawl_r));
    attr("ceiling_low", member(&Biped_Fixdefs::ceiling_low));
}
