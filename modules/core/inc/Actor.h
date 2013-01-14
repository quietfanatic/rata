#ifndef HAVE_CORE_ACTOR_H
#define HAVE_CORE_ACTOR_H

 // This should be moved to phys or somewhere eventually

struct Actor : Linkable<Actor> {
    virtual void act () { }
    virtual void react () { }
};

