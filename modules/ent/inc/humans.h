#ifndef HAVE_ENT_HUMANS_H
#define HAVE_ENT_HUMANS_H

#include "../../phys/inc/phys.h"
#include "../../phys/inc/aux.h"
#include "../../phys/inc/ground.h"
#include "../../geo/inc/rooms.h"
#include "../../vis/inc/models.h"
#include "../../vis/inc/sprites.h"
#include "../../core/inc/input.h"

namespace ent {

    struct Biped;
    struct BipedStats;
    struct BipedDef;

     // TODO: separate soul from body
    struct Biped : phys::Object, phys::Grounded, geo::Resident, vis::Draws_Sprites, core::Key_Listener {
         // Skeletons for bipeds must have these poses in this order.
        enum Pose {
            STAND,
            WALK1,
            WALK2,
            CRAWL1,
            CRAWL2_1,
            CRAWL2_2,
            HURTBK,
            LAYBK
        };
        struct Controls {
            bool left = false;
            bool right = false;
            bool jump = false;
            bool crouch = false;
        };

        BipedDef* def;
         // Bleh
        BipedDef* get_def () const { return def; }
        void set_def (BipedDef*);

        Controls controls;
        phys::Ambulator legs;
        vis::Model model;  // Must be a humanlike model
        int8 direction = 1;  // 1 = right, -1 = left
         // For animation purposes
        float distance_walked = 0;
        float oldxrel = 0;

        Biped ();

         // Do this after loading
        void finish ();

         // Key_Listener
        bool hear_key (int keycode, int action);

         // Make sure to supercall these if you override them.
        void draws_sprites ();
        void emerge ();
        void reclude ();
        void before_move ();
        void after_move ();

         // Primarily for internal use.
        bool allow_movement (BipedStats* stats, Controls* controls);
        bool allow_walk (BipedStats* stats, Controls* controls);
        bool allow_jump (BipedStats* stats, Controls* controls);
        bool allow_crouch (BipedStats* stats, Controls* controls);
        bool allow_crawl (BipedStats* stats, Controls* controls);
        bool allow_airmove (BipedStats* stats, Controls* controls);

    };

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

    struct BipedDef {
        phys::BodyDef* body_def;
        BipedStats* stats;  // Initial stats only.
        vis::Skel* skel;
        vis::Skin* skin;
    };

}

#endif
