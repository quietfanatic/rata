
#include "../inc/bipeds.h"
#include "../../hacc/inc/everything.h"
#include "../../util/inc/debug.h"

namespace ent {

     // Definitions
    void Biped::set_def (BipedDef* _def) {
        def = _def;
        apply_bdf(def->body_def);
        model.apply_skel(def->skel);
    }

     // Draws_Sprites
    void Biped::draws_sprites () {
        model.apply_skin(def->skin);
        if (ground) {
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
        else {
            model.apply_pose(&def->poses->walk1);
        }
        model.draw(pos(), direction < 0);
    }

     // Resident
    Vec Biped::resident_pos () {
        return pos();
    }
    void Biped::emerge () {
        materialize();
        legs.enable();
        Draws_Sprites::activate();
    }
    void Biped::reclude () {
        dematerialize();
        legs.disable();
        Draws_Sprites::deactivate();
    }

    void set_ambulate_friction (Biped* b, float fric) {
        b->legs.ambulate_force(
            b->b2body->GetMass()
          * -phys::space.get_gravity().y
          * sqrt(fric * b->get_ground_fix()->GetFriction())
        );
    }

    bool Biped::allow_movement () {
        if (ground) {
            legs.enable();
            allow_walk();
            allow_jump();
        }
        else {
            legs.disable();
            allow_airmove();
        }
        if (ground)
            oldxrel = pos().x - ground->pos().x;
        return true;
    }
    bool Biped::allow_walk () {
        if (ground) {
            if (buttons & LEFT_BIT && !(buttons & RIGHT_BIT)) {
                legs.ambulate_x(this, -stats.walk_speed);
                if (vel().x <= 0) {
                    direction = -1;
                    set_ambulate_friction(this, stats.walk_friction);
                }
                else {
                    set_ambulate_friction(this, stats.skid_friction);
                }
            }
            else if (buttons & RIGHT_BIT) {
                legs.ambulate_x(this, stats.walk_speed);
                if (vel().x >= 0) {
                    direction = 1;
                    set_ambulate_friction(this, stats.walk_friction);
                }
                else {
                    set_ambulate_friction(this, stats.skid_friction);
                }
            }
            else {
                set_ambulate_friction(this, stats.stop_friction);
                legs.ambulate_x(this, 0);
            }
            return fabs(vel().x) >= 0.01;
        }
        else return false;
    }
    bool Biped::allow_jump () {
        if (buttons & JUMP_BIT) {
            legs.disable();
             // I'd like to be able to do a mutual push between the ground and
             //  the character, but Box2D isn't well-equipped for that, because
             //  it doesn't do shock propagation.
            impulse(Vec(0, stats.jump_impulse));
            ground = NULL;
            return true;
        }
        else return false;
    }

    bool Biped::allow_crouch () {
        return false;
    }
    bool Biped::allow_crawl () {
        return false;
    }
    bool Biped::allow_airmove () {
        legs.disable();
         // If you were in a 2D platformer, you'd be able to push against the air too.
        if (buttons & LEFT_BIT && !(buttons & RIGHT_BIT)) {
            force(Vec(-stats.air_force, 0));
            return true;
        }
        else if (buttons & RIGHT_BIT) {
            force(Vec(stats.air_force, 0));
            return true;
        }
        else return false;
    }
     // Ensure we're in the right room and also calculate walk animation.
    void Biped::after_move () {
        reroom();
        if (ground) {
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
    }
    void Biped::before_move () {
        allow_movement();
        buttons = ButtonBits(0);
    }

     // Controllable
    void Biped::control_buttons (ButtonBits bits) {
        buttons = bits;
    }

    Biped::Biped () : legs(this) { }

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
    attr("oldxrel", member(&Biped::oldxrel).optional());
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
    attr("air_force", member(&BipedStats::air_force).optional());
    attr("air_speed", member(&BipedStats::air_speed).optional());
    attr("jump_impulse", member(&BipedStats::jump_impulse).optional());
HCB_END(BipedStats)

HCB_BEGIN(BipedDef)
    name("ent::BipedDef");
    attr("body_def", member(&BipedDef::body_def));
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
    attr("crawl1", member(&BipedPoses::crawl1));
    attr("crawl2_1", member(&BipedPoses::crawl2_1));
    attr("crawl2_2", member(&BipedPoses::crawl2_2));
    attr("hurtbk", member(&BipedPoses::hurtbk));
    attr("laybk", member(&BipedPoses::laybk));
HCB_END(BipedPoses)
