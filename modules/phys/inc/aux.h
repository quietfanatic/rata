#ifndef HAVE_PHYS_AUX_H
#define HAVE_PHYS_AUX_H


#include "phys.h"

namespace phys {

    struct Feet {
        b2Body* friction_body;
        b2FrictionJoint* friction_joint;

        Feet (Object* obj);

         // ambulate_x and ambulate_y set the friction_body's velocity in the other
         // axis to that of the main body, so call them every frame that velocity changes.
        void ambulate_x (Object*, float);
        void ambulate_y (Object*, float);
        void ambulate_both (Object*, Vec);
        void ambulate_force (float);

        void enable ();
        void disable ();
        ~Feet ();
        
    };

}

#endif
