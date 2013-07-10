#ifndef HAVE_PHYS_GROUND_H
#define HAVE_PHYS_GROUND_H

#include "phys.h"

 // Slightly annoying to have this in its own module, but
 //  since it registers a collision rule, we don't want to
 //  tie it into the base physics module.

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
