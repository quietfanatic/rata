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
        float jump_delay;
    };
    struct BipedPoses;
    struct BipedFixdefs;
    struct BipedDef;

    struct Biped : phys::Object, phys::Grounded, geo::Resident, vis::Sprites, Controllable {

        BipedDef* def;
         // Bleh
        BipedDef* get_def () const { return def; }
        void set_def (BipedDef*);

         // For control
        ButtonBits buttons = ButtonBits(0);
        Vec focus = Vec(2, 0);
        void Controllable_buttons (ButtonBits) override;
        void Controllable_move_focus (Vec) override;
        Vec Controllable_get_focus () override;
        int8 move_direction () {
            return !!(buttons & RIGHT_BIT) - !!(buttons & LEFT_BIT);
        }

         // For movement
        BipedStats stats;
        int8 direction = 1;
        bool crouching = false;
        bool crawling = false;
        bool ceiling_low = false;  // Established by a sensor
        uint8 jump_timer = 0;  // counts up until stats.jump_delay
        void before_move () override;  // Object
        void after_move () override;
        float Grounded_velocity () override;
        float Grounded_friction () override;

         // For animation
        vis::Model model;  // Must be a humanlike model
        float distance_walked = 0;
        float oldxrel = 0;
        void Sprites_draw (vis::Sprites_Renderer) override;


        Vec Resident_pos () override;
        void Resident_emerge () override;
        void Resident_reclude () override;

        Biped ();
        void finish ();

    };

    struct BipedPoses {
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
    };

    struct BipedFixdefs {
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

    struct BipedDef {
        phys::BodyDef* body_def;
        BipedFixdefs* fixdefs;
        BipedStats* stats;  // Initial stats only.
        BipedPoses* poses;
        vis::Skel* skel;
        vis::Skin* skin;
         // Not part of the frame pts, because it shouldn't vary.
        Vec focus_offset;
    };

}

#endif
