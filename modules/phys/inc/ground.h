#ifndef HAVE_PHYS_GROUND_H
#define HAVE_PHYS_GROUND_H

#include "phys.h"

 // This modules provides a collision rule "ground" that
 //  allows Grounded objects to walk using friction and surface
 //  velocity, kinda like you actually walk.

 // Notice: due to unfortunate ramifications between how this and the
 //  physics library work together, the ground collision rule cannot
 //  wake objects up when their ground velocity is going to change.
 //  So you must make sure to call b2body->SetAwake(true) on any
 //  frames your ground velocity is likely to change.

namespace phys {

    struct Grounded {
        Object* ground = NULL;
         // We can't record the b2Fixture* directly, since it
         //  has no path and cannot be saved to a file.
        FixtureDef* ground_fixdef = NULL;

         // Use these to walk.  The velocity is your ideal speed
         //  (positive -> right) and the friction is your acceleration
         //  or deceleration.  If friction returns NAN, the bodydef's normal
         //  friction value will be used.
        virtual float Grounded_velocity () { return 0; }
        virtual float Grounded_friction () { return NAN; }
    };

}

#endif
