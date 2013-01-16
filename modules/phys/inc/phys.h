#ifndef HAVE_PHYS_PHYS_H
#define HAVE_PHYS_PHYS_H

namespace phys {

    extern b2World* sim;

    struct FixtureDef {
        b2FixtureDef b2;

        b2Fixture* manifest (b2Body*);
    };

    struct BodyDef {
        b2BodyDef b2;
        std::vector<FixtureDef> fixtures;

        BodyDef () { b2.active = false; }

        b2Body* manifest (b2World* sim, Vec pos, Vec vel = Vec(0, 0));
    };

    struct Physical {
        b2Body* body = NULL;
        BodyDef* body_def;
         // All the information has to be cached because it's read before the body is created
        Vec pos;
        Vec vel;

        Vec pos () { return reinterpret_cast<const Vec&>(body->GetPosition()); }
        void set_pos (Vec v) { body->SetPosition(b2Vec2(v.x, v.y)); }
        Vec vel () { const b2Vec2& v = body->GetLinearVelocity(); return reinterpret_cast<const Vec&>(v); }
        void set_vel (Vec v) { body->SetLinearVelocity(b2Vec2(v.x, v.y)); }

        Physical (BodyDef* body_def) : body_def(body_def) { }
        void exist ();  // Registers with the sim.
    };

}

#endif
