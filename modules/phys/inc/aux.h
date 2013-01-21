#ifndef HAVE_PHYS_AUX_H
#define HAVE_PHYS_AUX_H

#include "phys.h"

namespace phys {

    struct Walking : Physical {
        b2Body* friction_body;
        b2FrictionJoint* friction_joint;

        Walking (BodyDef* bdf);

         // It is recommended to call these every frame.
        void set_walk_vel (float f);
        void set_walk_friction (float f);

        void activate () { Physical::activate(); friction_body->SetActive(true); }
        void deactivate () { Physical::deactivate(); friction_body->SetActive(false); }
        
        void start () {
            Physical::start();  // activate is virtual, so it calls Walking::activate
        }

    };

}

#endif
