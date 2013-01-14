#ifndef HAVE_PHYS_PHYS_H
#define HAVE_PHYS_PHYS_H

namespace phys {

     // This keeps track of all the state belonging to a physics simulation
     //  ...which is...absolutely nothing.
    struct Sim {
    };

    struct BodyInfo {
    };

     // This wraps a b2Body, and should be saved in the game state.
    struct Body : b2Body {
        const Vec& pos () { return reinterpret_cast<const Vec&>(GetPosition()); }
        void set_pos (Vec p) { SetPosition(reinterpret_cast<const Vec&>(p)); }
        const Vec& vel () { return reinterpret_cast<const Vec&>(GetLinearVelocity()); }
        void set_vel (Vec v) { SetLinearVelocity(reinterpret_cast<const Vec&>(v)); }
    };

    void init ();

    void step ();

}

#endif
