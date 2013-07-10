#ifndef HAVE_ENT_BIPEDS_H
#define HAVE_ENT_BIPEDS_H

#include "../inc/control.h"
#include "../../phys/inc/phys.h"
#include "../../phys/inc/ground.h"
#include "../../geo/inc/rooms.h"
#include "../../vis/inc/models.h"
#include "../../vis/inc/sprites.h"

namespace ent {

    struct Biped;
    struct BipedStats {
        float walk_friction;
        float walk_speed;
        float run_friction;
        float run_speed;
        float crawl_friction;
        float crawl_speed;
        float stop_friction;
        float skid_friction;
        float air_friction;
        float air_speed;
        float jump_speed;
    };
    struct BipedPoses;
    struct BipedFixdefs;
    struct BipedDef;

     // TODO: separate soul from body
    struct Biped : phys::Object, phys::Grounded, geo::Resident, vis::Sprite, Controllable {

        BipedDef* def;
         // Bleh
        BipedDef* get_def () const { return def; }
        void set_def (BipedDef*);

         // For movement
        BipedStats stats;
        int8 direction = 1;
        bool crouching = false;
        bool crawling = false;
         // Object
        void before_move () override;
        void after_move () override;
        float Grounded_velocity () override;
        float Grounded_friction () override;

         // For animation
        vis::Model model;  // Must be a humanlike model
        float distance_walked = 0;
        float oldxrel = 0;
        void Sprite_draw () override;

        ButtonBits buttons;
         // Controllable
        void control_buttons (ButtonBits) override;
        int8 move_direction () {
            return !!(buttons & RIGHT_BIT) - !!(buttons & LEFT_BIT);
        }

        Vec Resident_pos () override;
        void Resident_emerge () override;
        void Resident_reclude () override;

        Biped ();
        void finish ();

    };

    struct BipedPoses {
        vis::Pose stand;
        vis::Pose walk1;
        vis::Pose walk2;
        vis::Pose crouch;
        vis::Pose crawl1;
        vis::Pose crawl2_1;
        vis::Pose crawl2_2;
        vis::Pose hurtbk;
        vis::Pose laybk;
    };

    struct BipedFixdefs {
        phys::FixtureDef stand;
        phys::FixtureDef hurt;
        phys::FixtureDef crouch;
        phys::FixtureDef crawl_l;
        phys::FixtureDef crawl_r;
    };

    struct BipedDef {
        phys::BodyDef* body_def;
        BipedStats* stats;  // Initial stats only.
        BipedPoses* poses;
        vis::Skel* skel;
        vis::Skin* skin;
    };

}

#endif
