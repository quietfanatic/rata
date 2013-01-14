#ifndef HAVE_PHYS_PHYS_H
#define HAVE_PHYS_PHYS_H

namespace phys {

    extern b2World* sim;

    void init ();

    void step ();

    struct FixtureDef {
        b2FixtureDef b2;

        b2Fixture* manifest (b2Body*);
    };

    struct BodyDef {
        b2BodyDef b2;
        std::vector<FixtureDef> fixtures;

        b2Body* manifest (b2World* sim, Vec pos, Vec vel = Vec(0, 0));
    };

}

#endif
