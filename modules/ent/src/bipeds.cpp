#include "ent/inc/bipeds.h"

#include "geo/inc/camera.h"
#include "hacc/inc/everything.h"
#include "util/inc/debug.h"
#include "vis/inc/models.h"

namespace ent {

    void Biped::finish () {
        ROD::finish();
        auto def = get_def();
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
        focus = constrain(Rect(-18, -13, 18, 13), focus + diff);
    }
    Vec Biped::Controllable_get_focus () {
        return focus + pos() + get_def()->focus_offset;
    }
    Vec Biped::Controllable_get_pos () {
        return pos() + get_def()->focus_offset;
    }
    geo::Room* Biped::Controllable_get_room () {
        return room;
    }

     // Change some kinds of movement state
     // Do not change ground velocity, but do change air velocity
    void Biped::Object_before_move () {
        auto def = get_def();
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
    void Biped::Object_after_move () {
        auto def = get_def();
         // Clear input before next frame
        buttons = Button_Bits(0);
        reroom(pos());
        if (ground && (!crouching || crawling)) {
            if (fabs(vel().x) < 0.01) {
                distance_walked = 0;
            }
            else if (crawling) {
                distance_walked += fabs(pos().x - ground->pos().x - oldxrel);
            }
            else {
                float stride = fabs(vel().x) > stats.walk_speed
                    ? stats.run_stride
                    : stats.walk_stride;
                bool pre_step = fmod(distance_walked, stride / 2) < stride / 4;
                distance_walked += fabs(pos().x - ground->pos().x - oldxrel);
                if (pre_step && fmod(distance_walked, stride / 2) >= stride / 4) {
                    if (stats.step_voice) {
                        stats.step_voice->done = false;
                        stats.step_voice->paused = false;
                        stats.step_voice->pos = 0;
                        stats.step_voice->volume = 0.3 + 0.3 * fabs(vel().x) / stats.walk_speed;
                    }
                }
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
         // Vision update
        vision.set_focus(pos() + def->focus_offset + Vec(floor(focus.x/2/PX), floor(focus.y/2/PX))*PX);
        vision.look_at(pos() + def->focus_offset + focus + Rect(-1, -1, 1, 1), 1000000);
        Vec v_pos = vision.get_pos(!!controller);
         // Camera control
         // TODO: Put this in Player
        if (controller) {
            geo::default_camera().pos = v_pos;
        }
    }

    void Biped::Drawn_draw (vis::Sprites) {
        auto def = get_def();
        char model_data [def->skel->model_data_size()];
        vis::Model model (def->skel, model_data);
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
                    uint step = fmod(distance_walked / stats.crawl_stride * 4.0, 4.0);
                    model.apply_pose(&def->poses->crawl[step]);
                }
            }
            else if (crouching) {
                model.apply_pose(&def->poses->crouch);
                model.apply_pose(&def->poses->look_stand[look_frame]);
            }
            else if (vel().x * direction > stats.walk_speed) {
                float stepdist = fmod(distance_walked / stats.run_stride * 6.0, 6.0);
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
                uint step = fmod(distance_walked / stats.walk_stride * 4.0, 4.0);
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
         // TODO: implement this as an item
        size_t n_skins = 1 + equipment.items.count();
        vis::Skin* skins [n_skins];
        skins[0] = def->skin;
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
    attr("ROD", base<ROD<vis::Sprites, Biped_Def>>().collapse());
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
    attr("walk_stride", member(&Biped_Stats::walk_stride).optional());
    attr("run_stride", member(&Biped_Stats::run_stride).optional());
    attr("crawl_stride", member(&Biped_Stats::crawl_stride).optional());
    attr("step_voice", member(&Biped_Stats::step_voice).optional());
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
