#ifndef HAVE_PHYS_PHYS_H
#define HAVE_PHYS_PHYS_H

#include "../../core/inc/state.h"

namespace phys {

    extern b2World* sim;

    void init ();

    struct FixtureDef {
        b2FixtureDef b2;

        b2Fixture* manifest (b2Body*);
    };

    struct BodyDef {
        b2BodyDef b2;
        std::vector<FixtureDef> fixtures;

        b2Body* manifest (b2World* sim, Vec pos, Vec vel = Vec(0, 0));
    };

    struct Physical {
        b2Body* body = NULL;
        BodyDef* body_def;
        Physical (BodyDef* body_def) : body_def(body_def) { }
        void exist ();  // Registers with the sim.
    };

}

#endif
