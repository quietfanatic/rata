#ifndef HAVE_ENT_BIPEDS_H
#define HAVE_ENT_BIPEDS_H

#include "ent/inc/control.h"
#include "ent/inc/items.h"
#include "ent/inc/mixins.h"
#include "geo/inc/rooms.h"
#include "geo/inc/vision.h"
#include "phys/inc/ground.h"
#include "phys/inc/phys.h"
#include "snd/inc/audio.h"
#include "vis/inc/common.h"
#include "vis/inc/models.h"

namespace ent {

    struct Biped_Stats {
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
        float jump_delay;
        float walk_stride;
        float run_stride;
        float crawl_stride;
        snd::Voice* step_voice = NULL;
    };

     // Enumerates all possible poses for a Biped.
    struct Biped_Poses {
        vis::Pose stand;
        vis::Pose walk [4];
        vis::Pose run [6];
        vis::Pose crouch;
        vis::Pose prejump;
        vis::Pose jump;
        vis::Pose crawl [4];
        vis::Pose hurtbk;
        vis::Pose laybk;
        vis::Pose look_stand [9];
        vis::Pose look_walk [9];
        vis::Pose aim_f [9];
    };

     // All the fixdefs for a given Biped.
    struct Biped_Fixdefs {
        phys::FixtureDef feet;
        phys::FixtureDef stand;
        phys::FixtureDef hurt;
        phys::FixtureDef crouch;
        phys::FixtureDef crawl_l;
        phys::FixtureDef crawl_r;
        phys::FixtureDef ceiling_low;
         // LOL, categorizing things by comparing their addresses
        bool is_primary (phys::FixtureDef* fd) {
            return fd >= &stand && fd <= &crawl_r;
        }
        bool is_sensor (phys::FixtureDef* fd) {
            return fd == &ceiling_low;
        }
    };

     // Various bits of static info.
    struct Biped_Def : Agent_Def {
        Biped_Fixdefs* fixdefs;
        Biped_Stats* stats;  // Initial stats only.
        Biped_Poses* poses;
        vis::Skel* skel;
        vis::Skin* skin;
    };

    struct Biped : Agent<vis::Sprites, Biped_Def>, phys::Grounded {

         // For control
        uint32 buttons = 0;
        Vec focus = Vec(2, 0);
        Vec vision_pos;
        int8 move_direction () {
            return !!(buttons & RIGHT_BIT) - !!(buttons & LEFT_BIT);
        }

        geo::Vision vision;

         // For movement
        Biped_Stats stats;
        int8 direction = 1;
        bool crouching = false;
        bool crawling = false;
        bool ceiling_low = false;  // Established by a sensor
        uint8 jump_timer = 0;  // counts up until stats.jump_delay

        void Object_before_move () override;
        void Object_after_move () override;
        float Grounded_velocity () override;
        float Grounded_friction () override;

         // Items, equipment
        Inventory equipment;
        Inventory inventory;
        Item* hand_item ();  // NULL if none
        uint attack_timeout = 0;

        Vec model_seg_point (vis::Skel::Seg*, size_t point_i);

         // For animation
        float distance_walked = 0;
        float oldxrel = 0;
        bool aiming = false;
        void Drawn_draw (vis::Sprites) override;

         // Internal, used for both drawing and model_seg_point
        void animate (vis::Model*);

        Biped ();

        void finish ();

    };

}

#endif
