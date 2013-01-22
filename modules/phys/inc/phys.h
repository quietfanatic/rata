#ifndef HAVE_PHYS_PHYS_H
#define HAVE_PHYS_PHYS_H

#include <Box2D/Box2D.h>
#include "../../util/inc/Vec.h"
#include "../../util/inc/organization.h"
#include "../../hacc/inc/haccable_integration.h"

namespace phys {
    struct Object;

    extern b2World* space;

     // Static things, probably stored in files

    struct FixtureDef {
        std::string name;
        b2FixtureDef b2;
        uint64 collidable_a = 0;
        uint64 collidable_b = 0;

        b2Fixture* manifest (b2Body*);
    };
    struct BodyDef {
        b2BodyDef b2;
        std::vector<FixtureDef> fixtures;

        BodyDef () { b2.active = false; }

        b2Body* manifest (Object* owner, b2World* space, Vec pos = Vec(0, 0), Vec vel = Vec(0, 0));
    };

     // The dynamic thing

    struct Object {
        b2Body* b2body = NULL;

        Vec pos () const { return reinterpret_cast<const Vec&>(b2body->GetPosition()); }
        void set_pos (Vec v) { b2body->SetTransform(b2Vec2(v.x, v.y), 0); }
        Vec vel () const { const b2Vec2& v = b2body->GetLinearVelocity(); return reinterpret_cast<const Vec&>(v); }
        void set_vel (Vec v) { b2body->SetLinearVelocity(b2Vec2(v.x, v.y)); }
        void impulse (Vec i) { b2body->ApplyLinearImpulse(b2Vec2(i.x, i.y), b2Vec2(0, 0)); }

        void materialize ();
        void dematerialize ();

        virtual void before_move () { }
        virtual void after_move () { }
        virtual void while_intangible () { }
        virtual ~Object () { if (b2body) space->DestroyBody(b2body); }

        Object (BodyDef* body_def) { b2body = body_def->manifest(this, space); }
    };

     // Collision handling is done through registered collision rules.
     // A maximum of 64 collision rules can be registered.
    
    struct Collision_Rule {
        uint index;
        virtual std::string name () const = 0;
        virtual void post (b2Contact*, b2Fixture*, b2Fixture*) { }
        virtual void end (b2Contact*, b2Fixture*, b2Fixture*) { }
        virtual ~Collision_Rule () { }  // To make the compiler happy, mostly

        Collision_Rule ();

        uint64 bit () { return 1 << index; }
    };

}

#endif
