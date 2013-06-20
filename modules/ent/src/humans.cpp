
#include "../../hacc/inc/everything.h"
#include "../inc/humans.h"
#include "../../core/inc/commands.h"

namespace ent {

    void Biped::set_def (BipedDef* _def) {
        def = _def;
        apply_bdf(def->body_def);
        model.apply_skel(def->skel);
    }

    void Biped::draws_sprites () {
        model.apply_skin(def->skin);
        if (ground) {
            if (fabs(vel().x) < 0.01) {
                model.apply_pose_index(Pose::STAND);
            }
            else {
                float stepdist = fmod(distance_walked, 2.0);
                if (stepdist < 0.5) {
                    model.apply_pose_index(Pose::WALK1);
                }
                else if (stepdist >= 1 && stepdist < 1.5) {
                    model.apply_pose_index(Pose::WALK2);
                }
                else {
                    model.apply_pose_index(Pose::STAND);
                }
            }
        }
        else {
            model.apply_pose_index(Pose::WALK1);
        }
        model.draw(pos(), direction < 0);
    }

    bool Biped::allow_movement (BipedStats* stats, Biped::Controls* controls) {
        if (ground) {
            legs.enable();
            allow_walk(stats, controls);
            allow_jump(stats, controls);
        }
        else {
            allow_airmove(stats, controls);
        }
        if (ground)
            oldxrel = pos().x - ground->pos().x;
        return true;
    }
    void Biped::emerge () { materialize(); legs.enable(); Draws_Sprites::activate(); }
    void Biped::reclude () { dematerialize(); legs.disable(); Draws_Sprites::deactivate(); }

    void set_ambulate_friction (Biped* b, float fric) {
        b->legs.ambulate_force(
            b->b2body->GetMass()
          * -phys::space->get_gravity().y
          * sqrt(fric * b->get_ground_fix()->GetFriction())
        );
    }

    bool Biped::allow_walk (BipedStats* stats, Biped::Controls* controls) {
        if (ground) {
            legs.enable();
            if (controls->left && !controls->right) {
                legs.ambulate_x(this, -stats->walk_speed);
                if (vel().x <= 0) {
                    direction = -1;
                    set_ambulate_friction(this, stats->walk_friction);
                }
                else {
                    set_ambulate_friction(this, stats->skid_friction);
                }
            }
            else if (controls->right) {
                legs.ambulate_x(this, stats->walk_speed);
                if (vel().x >= 0) {
                    direction = 1;
                    set_ambulate_friction(this, stats->walk_friction);
                }
                else {
                    set_ambulate_friction(this, stats->skid_friction);
                }
            }
            else {
                set_ambulate_friction(this, stats->stop_friction);
                legs.ambulate_x(this, 0);
            }
            return fabs(vel().x) >= 0.01;
        }
        else return false;
    }
    bool Biped::allow_jump (BipedStats* stats, Biped::Controls* controls) {
        if (controls->jump) {
            legs.disable();
             // I'd like to be able to do a mutual push between the ground and
             //  the character, but Box2D isn't well-equipped for that, because
             //  it doesn't do shock propagation.
            impulse(Vec(0, stats->jump_impulse));
            ground = NULL;
            return true;
        }
        else return false;
    }

    bool Biped::allow_crouch (BipedStats* stats, Biped::Controls* controls) {
        return false;
    }
    bool Biped::allow_crawl (BipedStats* stats, Biped::Controls* controls) {
        return false;
    }
    bool Biped::allow_airmove (BipedStats* stats, Biped::Controls* controls) {
        legs.disable();
         // If you were in a 2D platformer, you'd be able to push against the air too.
        if (controls->left && !controls->right) {
            force(Vec(-stats->air_force, 0));
            return true;
        }
        else if (controls->right) {
            force(Vec(stats->air_force, 0));
            return true;
        }
        else return false;
    }
     // Ensure we're in the right room and also calculate walk animation.
    void Biped::after_move () {
        reroom(pos());
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
        allow_movement(def->stats, &controls);
    }

    Biped::Biped () : legs(this) { }

    bool Biped::hear_key (int keycode, int action) {
        bool on = action == GLFW_PRESS;
        switch (keycode) {
            case 'A': controls.left = on; return true;
            case 'D': controls.right = on; return true;
            case 'W': controls.jump = on; return true;
            case 'S': controls.crouch = on; return true;
            default: return false;
        }
    }

    void Biped::start () {
        geo::geography->behold(this); // TODO this doesn't belong here
    }

} using namespace ent;

HCB_BEGIN(Biped)
    type_name("ent::Biped");
    base<core::Stateful>("Biped");
    attr("def", value_methods(&Biped::get_def, &Biped::set_def)(required));
    attr("object", supertype<phys::Object>());
    attr("resident", supertype<geo::Resident>());
    attr("grounded", supertype<phys::Grounded>());
    attr("direction", member(&Biped::direction)(1));
    attr("distance_walked", member(&Biped::distance_walked)(0));
    attr("oldxrel", member(&Biped::oldxrel)(0));
HCB_END(Biped)

HCB_BEGIN(BipedStats)
    type_name("ent::BipedStats");
    attr("walk_friction", member(&BipedStats::walk_friction)(1));
    attr("walk_speed", member(&BipedStats::walk_speed)(1));
    attr("run_friction", member(&BipedStats::run_friction)(1));
    attr("run_speed", member(&BipedStats::run_speed)(1));
    attr("crawl_friction", member(&BipedStats::crawl_friction)(1));
    attr("crawl_speed", member(&BipedStats::crawl_speed)(1));
    attr("stop_friction", member(&BipedStats::stop_friction)(1));
    attr("skid_friction", member(&BipedStats::skid_friction)(1));
    attr("air_force", member(&BipedStats::air_force)(1));
    attr("air_speed", member(&BipedStats::air_speed)(1));
    attr("jump_impulse", member(&BipedStats::jump_impulse)(1));
HCB_END(BipedStats)

HCB_BEGIN(BipedDef)
    type_name("ent::BipedDef");
    attr("body_def", member(&BipedDef::body_def)(required));
    attr("stats", member(&BipedDef::stats)(required));
    attr("skel", member(&BipedDef::skel)(required));
    attr("skin", member(&BipedDef::skin)(required));
HCB_END(BipedDef)
