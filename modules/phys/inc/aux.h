#ifndef HAVE_PHYS_AUX_H
#define HAVE_PHYS_AUX_H

#include "phys.h"

namespace phys {

    struct Ambulator : Object {
        b2Body* friction_body;
        b2FrictionJoint* friction_joint;

        Ambulator (BodyDef* bdf);

         // ambulate_x and ambulate_y set the friction_body's velocity in the other
         // axis to that of the main body, so call them every frame that velocity changes.
        void ambulate_x (float x);
        void ambulate_y (float y);
        void ambulate_both (Vec v);
        void ambulate_force (float f);

        void materialize ();
        void dematerialize ();
        ~Ambulator () { dematerialize(); }
        
    };

}

#endif
