#ifndef HAVE_PHYS_PHYS_H
#define HAVE_PHYS_PHYS_H

#include <Box2D/Box2D.h>
#include "../../util/inc/geometry.h"

namespace phys {
    using namespace util;

    struct Object;

     // This contains all physical objects and provides access to the b2World.
    struct Space {
        b2World* b2world;
        Space ();

        Vec get_gravity () const { return b2world->GetGravity(); }
        void set_gravity (Vec g) { b2world->SetGravity(g); }

        void start ();
        void run ();
        void stop ();

        ~Space ();
    };
    extern Space space;

     // Box2D's default filtering system is really weird.  We're gonna
     //  use our own.  We can reinterpret the b2Filter that's on every
     //  fixture however we want, as long as we stay limited to 48 bits.
    struct Filter {
        uint16 mask = 1;  // If two fixtures share mask bits, they collide,
        uint16 unmask = 0;  // unless they also share unmask bits.
        bool active = true;  // Inactive fixtures do not collide.
        Filter () { }
        Filter (const b2Filter& b2f) : Filter(reinterpret_cast<const Filter&>(b2f)) { }
        operator b2Filter& () { return reinterpret_cast<b2Filter&>(*this); }
        operator const b2Filter& () const { return reinterpret_cast<const b2Filter&>(*this); }
    };


     // Every physics interface has to decide which properties of objects are dynamic and
     //  which are static.  We are making a lot more properties static than Box2D does.

     // Stores shape, friction, collision behavior data.
    struct FixtureDef {
        b2FixtureDef b2;
        uint64 coll_a = 0;
        uint64 coll_b = 0;
        Filter filter;  // Just the initial filter
    };
     // Stores properties of motion, inertia, and a list of fixtures.
    struct BodyDef {
        b2BodyType type = b2_dynamicBody;
        float mass = NAN;
        float damping = 0;
        float gravity_scale = 1;
        std::vector<FixtureDef*> fixtures;
    };

     // The dynamic thing
     // Every class that wants to have a physical presence should inherit from this.
    struct Object {
        b2Body* b2body = NULL;

         // A paltry amount of wrapper methods.
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
        void foreach_contact (const std::function<void (b2Fixture* mine, b2Fixture* other)>&);

         // These activate and deactivate the b2Body
        void materialize ();
        void dematerialize ();

         // Called every frame before space simulation, only if tangible
        virtual void before_move () { }
         // Called every frame after space simulation, only if tangible
        virtual void after_move () { }
         // Called every frame after space simulation, only if not tangible
        virtual void while_intangible () { }


        Object ();
        virtual ~Object ();
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
