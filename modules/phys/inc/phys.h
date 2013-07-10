#ifndef HAVE_PHYS_PHYS_H
#define HAVE_PHYS_PHYS_H

#include <Box2D/Box2D.h>
#include "../../core/inc/phases.h"
#include "../../util/inc/Vec.h"

namespace phys {

    struct Object;

    struct Space : core::Phase {
        b2World* b2world;
        Space ();

        Vec get_gravity () const { return b2world->GetGravity(); }
        void set_gravity (Vec g) { b2world->SetGravity(g); }

        void Phase_start () override;
        void Phase_run () override;
        ~Space ();
    };
    extern Space space;

     // Static things, probably stored in files

    struct FixtureDef {
        b2FixtureDef b2;
        uint64 coll_a = 0;
        uint64 coll_b = 0;
    };
    struct BodyDef {
        b2BodyType type = b2_dynamicBody;
        float mass = NAN;
        float damping = 0;
        float gravity_scale = 1;
        std::vector<FixtureDef*> fixtures;
    };

     // The dynamic thing

    struct Object {
        b2Body* b2body = NULL;

        Vec pos () const { return reinterpret_cast<const Vec&>(b2body->GetPosition()); }
        void set_pos (Vec v) { b2body->SetTransform(b2Vec2(v.x, v.y), 0); }
        Vec vel () const { const b2Vec2& v = b2body->GetLinearVelocity(); return reinterpret_cast<const Vec&>(v); }
        void set_vel (Vec v) { b2body->SetLinearVelocity(b2Vec2(v.x, v.y)); }
        void impulse (Vec i) { b2body->ApplyLinearImpulse(b2Vec2(i.x, i.y), b2Vec2(0, 0), true); }
        void force (Vec f) { b2body->ApplyForceToCenter(b2Vec2(f.x, f.y), true); }

        void apply_bdf (BodyDef*);
        b2Fixture* add_fixture (FixtureDef*);

        b2Fixture* get_fixture (FixtureDef* fd);

         // For big convenience
        void foreach_contact (std::function<void (b2Fixture* mine, b2Fixture* other)>&);

         // These activate and deactivate the b2Body
        void materialize ();
        void dematerialize ();

        virtual void before_move () { }
        virtual void after_move () { }
        virtual void while_intangible () { }
        virtual ~Object () { if (b2body) space.b2world->DestroyBody(b2body); }

        Object ();
    };

     // Collision handling is done through registered collision rules.
     // A maximum of 64 collision rules can be registered.

    struct Collision_Rule {
        uint index;
        virtual std::string name () const = 0;
        virtual void Collision_Rule_presolve (b2Contact*, b2Fixture*, b2Fixture*) { }
        virtual void Collision_Rule_end (b2Contact*, b2Fixture*, b2Fixture*) { }
        virtual ~Collision_Rule () { }  // To make the compiler happy, mostly

        Collision_Rule ();

        uint64 bit () { return 1 << index; }
    };

}

#endif
