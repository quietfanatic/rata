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
    };

}

#endif
