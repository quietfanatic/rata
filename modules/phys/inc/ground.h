#ifndef HAVE_PHYS_GROUND_H
#define HAVE_PHYS_GROUND_H

#include "phys.h"

 // Slightly annoying to have this in its own module, but
 //  since it registers a collision rule, we don't want to
 //  tie it into the base physics module.

namespace phys {

    struct Grounded {
        Object* ground = NULL;
        uint ground_fix_index = 0;
        b2Fixture* get_ground_fix () {
            if (ground)
                return ground->fix_no(ground_fix_index);
            else return NULL;
        }
    };
      // There's no required behavior for an object to be ground.

}

#endif
