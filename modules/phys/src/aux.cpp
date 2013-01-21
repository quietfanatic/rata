
#include "../inc/aux.h"

namespace phys {

    Walking::Walking (BodyDef* bdf) : Physical(bdf) {
        b2BodyDef b2bdf;
        b2bdf.type = b2_kinematicBody;
        b2bdf.gravityScale = 0;
        b2bdf.fixedRotation = true;
        friction_body = sim->CreateBody(&b2bdf);
        b2FrictionJointDef b2jdf;
        b2jdf.bodyA = body;
        b2jdf.bodyB = friction_body;
        b2jdf.maxForce = 0;
        friction_joint = (b2FrictionJoint*)sim->CreateJoint(&b2jdf);
    }
    void Walking::set_walk_vel (float f) {
        friction_body->SetTransform(b2Vec2(0, 0), 0);
        friction_body->SetLinearVelocity(b2Vec2(f, vel().y));
    }
    void Walking::set_walk_friction (float f) {
        friction_joint->SetMaxForce(f);
    }

}
