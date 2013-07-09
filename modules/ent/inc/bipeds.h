#ifndef HAVE_ENT_BIPEDS_H
#define HAVE_ENT_BIPEDS_H

#include "../inc/control.h"
#include "../../phys/inc/phys.h"
#include "../../phys/inc/aux.h"
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
        float air_force;
        float air_speed;
        float jump_impulse;
    };
    struct BipedPoses;
    struct BipedDef;

     // TODO: separate soul from body
    struct Biped : phys::Object, phys::Grounded, geo::Resident, vis::Draws_Sprites, Controllable {

        BipedDef* def;
         // Bleh
        BipedDef* get_def () const { return def; }
        void set_def (BipedDef*);

         // For movement
        phys::Ambulator legs;
        BipedStats stats;
        int8 direction = 1;  // 1 = right, -1 = left
         // Object
        void before_move ();
        void after_move ();

         // For animation
        vis::Model model;  // Must be a humanlike model
        float distance_walked = 0;
        float oldxrel = 0;
         // Draws_Sprites
        void draws_sprites ();

        ButtonBits buttons;
         // Controllable
        void control_buttons (ButtonBits);

         // Resident
        Vec resident_pos ();
        void emerge ();
        void reclude ();

         // Primarily for internal use.
        bool allow_movement ();
        bool allow_walk ();
        bool allow_jump ();
        bool allow_crouch ();
        bool allow_crawl ();
        bool allow_airmove ();

        Biped ();
        void finish ();

    };

    struct BipedPoses {
        vis::Pose stand;
        vis::Pose walk1;
        vis::Pose walk2;
        vis::Pose crawl1;
        vis::Pose crawl2_1;
        vis::Pose crawl2_2;
        vis::Pose hurtbk;
        vis::Pose laybk;
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
