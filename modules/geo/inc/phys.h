#ifndef HAVE_GEO_PHYS_H
#define HAVE_GEO_PHYS_H

#include <functional>
#include <string>
#include <vector>
#include <Box2D/Box2D.h>
#include "geo/inc/spatial.h"
#include "util/inc/geometry.h"

namespace geo {

    struct Object;

     // Box2D's default filtering system is really weird.  We're gonna
     //  use our own.  We can reinterpret the b2Filter that's on every
     //  fixture however we want, as long as we stay limited to 48 bits.
    struct Filter {
        uint16 mask;  // If two fixtures share mask bits, they collide,
        uint16 unmask;  // unless they also share unmask bits.
        bool active;  // Inactive fixtures do not collide.
        Filter (uint16 m = 1, uint16 u = 0, bool a = true) : mask(m), unmask(u), active(a) { }
        Filter (const b2Filter& b2f) : Filter(reinterpret_cast<const Filter&>(b2f)) { }
        operator b2Filter& () { return reinterpret_cast<b2Filter&>(*this); }
        operator const b2Filter& () const { return reinterpret_cast<const b2Filter&>(*this); }
        bool test (const Filter&) const;
    };

     // This contains all physical objects and provides access to the b2World.
    struct Space {
        b2World* b2world;
        Space ();

        util::Vec get_gravity () const { return b2world->GetGravity(); }
        void set_gravity (util::Vec g) { b2world->SetGravity(g); }

        void start ();
        void run ();
        void stop ();
         // Individual phases of run, in case you want to interleave them with other stuff
        void run_before ();
        void run_simulation ();
        void run_after ();

         // Do a ray cast with a function instead of a class.  See Box2D API reference.
        typedef std::function<float (b2Fixture*, const util::Vec&, const util::Vec&, float)> RayCaster;
        void ray_cast (util::Vec start, util::Vec end, const RayCaster& f);

         // Query a shape at a position.
        typedef std::function<bool (b2Fixture*)> ShapeTester;
        bool query_shape (util::Vec pos, b2Shape* shape, const ShapeTester& f = nullptr, const Filter& filter = Filter(0xffff));

        ~Space ();
    };
    extern Space space;

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
     // Inherit from this to store more static properties of your objects.
    struct Object_Def {
        b2BodyType type = b2_dynamicBody;
        float mass = 0;
        float damping = 0;
        float gravity_scale = 1;
        std::vector<FixtureDef> fixtures;
    };

     // The dynamic thing
     // Every class that wants to have a physical presence should inherit from this.
    struct Object : virtual Spatial {
        Object_Def* def = NULL;
        b2Body* b2body = NULL;

        void finish ();

         // A paltry amount of wrapper methods.
        util::Vec Object_get_pos () const {
            return reinterpret_cast<const util::Vec&>(b2body->GetPosition());
        }
        util::Vec Spatial_get_pos () override { return Object_get_pos(); }
        void Object_set_pos (util::Vec v) {
            b2body->SetTransform(b2Vec2(v.x, v.y), 0);
            b2body->SetAwake(true);
        }
        void Spatial_set_pos (util::Vec v) override { Object_set_pos(v); }
        util::Vec get_vel () const {
            const b2Vec2& v = b2body->GetLinearVelocity();
            return reinterpret_cast<const util::Vec&>(v);
        }
        void set_vel (util::Vec v) {
            b2body->SetLinearVelocity(b2Vec2(v.x, v.y));
            b2body->SetAwake(true);
        }
        void impulse (util::Vec i) { b2body->ApplyLinearImpulse(b2Vec2(i.x, i.y), b2Vec2(0, 0), true); }
        void force (util::Vec f) { b2body->ApplyForceToCenter(b2Vec2(f.x, f.y), true); }

        b2Fixture* add_fixture (FixtureDef*);

        b2Fixture* get_fixture (FixtureDef* fd);

         // For big convenience
        void foreach_contact (const std::function<void (b2Fixture* mine, b2Fixture* other)>&);

         // These activate and deactivate the b2Body
        void materialize ();
        void dematerialize ();

         // Called every frame before space simulation, only if tangible
        virtual void Object_before_move () { }
         // Called every frame after space simulation, only if tangible
        virtual void Object_after_move () { }
         // Called every frame after space simulation, only if not tangible
        virtual void Object_while_intangible () { }

        Object ();
        virtual ~Object ();
        Object (const Object&) = delete;
        Object& operator = (const Object&) = delete;
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
