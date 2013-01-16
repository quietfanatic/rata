#ifndef HAVE_PHYS_PHYS_H
#define HAVE_PHYS_PHYS_H

#include <Box2D/Box2D.h>
#include "../../util/inc/Vec.h"
#include "../../util/inc/organization.h"

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

        b2Body* manifest (b2World* sim, Vec pos = Vec(0, 0), Vec vel = Vec(0, 0));
    };

    struct Actor : Linkable<Actor> {
        virtual Vec pos () const = 0;
//        virtual Room* room ();
        virtual void activate () { }
        virtual void deactivate () { }
        virtual void act () { }
        virtual void react () { }
        virtual void away () { }
        virtual ~Actor () { }

        void start ();
    };

     // This will be replaced by per-room lists.
    extern Links<Actor> all_actors;

    struct Physical : Actor {
        b2Body* body = NULL;

        Vec pos () const { return reinterpret_cast<const Vec&>(body->GetPosition()); }
        void set_pos (Vec v) { body->SetTransform(b2Vec2(v.x, v.y), 0); }
        Vec vel () const { const b2Vec2& v = body->GetLinearVelocity(); return reinterpret_cast<const Vec&>(v); }
        void set_vel (Vec v) { body->SetLinearVelocity(b2Vec2(v.x, v.y)); }

        Physical (BodyDef* body_def) { body = body_def->manifest(sim); }
        void activate ();
        void deactivate ();
        void start () { Actor::start(); activate(); }
    };

}

#endif
