#ifndef HAVE_ENT_HUMANS_H
#define HAVE_ENT_HUMANS_H

#include "../../phys/inc/phys.h"
#include "../../phys/inc/aux.h"
#include "../../phys/inc/ground.h"
#include "../../geo/inc/rooms.h"
#include "../../vis/inc/models.h"
#include "../../vis/inc/sprites.h"

namespace ent {

    struct Biped : phys::Object, phys::Grounded, geo::Resident, vis::Draws_Sprites {
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
        struct Stats {
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
        struct Controls {
            bool left;
            bool right;
            bool jump;
            bool crouch;
        };

        phys::Ambulator legs;
        vis::Model model;  // Must be a humanlike model
        int8 direction;  // 1 = right, -1 = left

        Biped (phys::BodyDef*, vis::Skel*);

         // Make sure to supercall these if you override them.
        void draw ();
        void emerge ();
        void reclude ();
        void after_move ();
         // You must override Object's before_move and call allow_movement.

         // Call this in before_move.
        bool allow_movement (Stats* stats, Controls* controls);
         // Primarily for internal use.
        bool allow_walk (Stats* stats, Controls* controls);
        bool allow_jump (Stats* stats, Controls* controls);
        bool allow_crouch (Stats* stats, Controls* controls);
        bool allow_crawl (Stats* stats, Controls* controls);
        bool allow_airmove (Stats* stats, Controls* controls);


    };

}

#endif
