
#include "../../hacc/inc/everything.h"
#include "../inc/humans.h"

namespace ent {

    void Biped::draw () {
        if (ground)
            model.apply_pose_index(Pose::STAND);
        else
            model.apply_pose_index(Pose::WALK1);
        model.draw(pos(), direction < 0);
    }

    bool Biped::allow_movement (Biped::Stats* stats, Biped::Controls* controls) {
        if (ground) {
            allow_walk(stats, controls);
            allow_jump(stats, controls);
        }
        else {
            allow_airmove(stats, controls);
        }
        return true;
    }
    void Biped::emerge () { materialize(); legs.enable(); appear(); }
    void Biped::reclude () { dematerialize(); legs.disable(); disappear(); }
    void Biped::after_move () { reroom(pos()); }

    bool Biped::allow_walk (Biped::Stats* stats, Biped::Controls* controls) {
        if (controls->left && !controls->right) {
            legs.ambulate_x(this, stats->walk_speed);
            if (vel().x <= 0) direction = -1;
            if (vel().x < -stats->walk_speed)
                legs.ambulate_force(stats->walk_force);
            else legs.ambulate_force(stats->skid_force);
        }
        else if (controls->right) {
            legs.ambulate_x(this, -stats->walk_speed);
            if (vel().x >= 0) direction = 1;
            if (vel().x > stats->walk_speed)
                legs.ambulate_force(stats->walk_force);
            else legs.ambulate_force(stats->skid_force);
        }
        else {
            legs.ambulate_force(stats->stop_force);
            legs.ambulate_x(this, 0);
        }
        return fabs(vel().x) >= 0.01;
    }
    bool Biped::allow_jump (Biped::Stats* stats, Biped::Controls* controls) {
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

    bool Biped::allow_crouch (Biped::Stats* stats, Biped::Controls* controls) {
        return false;
    }
    bool Biped::allow_crawl (Biped::Stats* stats, Biped::Controls* controls) {
        return false;
    }
    bool Biped::allow_airmove (Biped::Stats* stats, Biped::Controls* controls) {
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

    Biped::Biped (phys::BodyDef* bdf, vis::Skel* skel) :
        phys::Object(bdf), legs(this), model(skel)
    { }

}

using namespace ent;
HCB_BEGIN(Biped)
    type_name("ent::Biped");
    attr("object", supertype<phys::Object>());
    attr("resident", supertype<geo::Resident>());
    attr("grounded", supertype<phys::Grounded>());
    attr("direction", member(&Biped::direction, def((int8)1)));
HCB_END(Biped)

HCB_BEGIN(Biped::Stats)
    type_name("ent::Biped::Stats");
    attr("walk_force", member(&Biped::Stats::walk_force, def((float)1)));
    attr("walk_speed", member(&Biped::Stats::walk_speed, def((float)1)));
    attr("run_force", member(&Biped::Stats::run_force, def((float)1)));
    attr("run_speed", member(&Biped::Stats::run_speed, def((float)1)));
    attr("crawl_force", member(&Biped::Stats::crawl_force, def((float)1)));
    attr("crawl_speed", member(&Biped::Stats::crawl_speed, def((float)1)));
    attr("stop_force", member(&Biped::Stats::stop_force, def((float)1)));
    attr("skid_force", member(&Biped::Stats::skid_force, def((float)1)));
    attr("air_force", member(&Biped::Stats::air_force, def((float)1)));
    attr("air_speed", member(&Biped::Stats::air_speed, def((float)1)));
    attr("jump_impulse", member(&Biped::Stats::jump_impulse, def((float)1)));
HCB_END(Biped::Stats)

