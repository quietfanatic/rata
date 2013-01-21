
#include "../inc/aux.h"

namespace phys {

    Ambulator::Ambulator (BodyDef* bdf) : Object(bdf) {
        b2BodyDef b2bdf;
        b2bdf.type = b2_kinematicBody;
        b2bdf.gravityScale = 0;
        b2bdf.fixedRotation = true;
        friction_body = space->CreateBody(&b2bdf);

        b2FrictionJointDef b2jdf;
        b2jdf.bodyA = b2body;
        b2jdf.bodyB = friction_body;
        b2jdf.maxForce = 0;
        b2jdf.maxTorque = 0;
        friction_joint = (b2FrictionJoint*)space->CreateJoint(&b2jdf);
    }
    void Ambulator::ambulate_x (float x) {
        friction_body->SetTransform(pos(), 0);
        friction_body->SetLinearVelocity(b2Vec2(x, vel().y));
    }
    void Ambulator::ambulate_y (float y) {
        friction_body->SetTransform(pos(), 0);
        friction_body->SetLinearVelocity(b2Vec2(vel().x, y));
    }
    void Ambulator::ambulate_both (Vec v) {
        friction_body->SetTransform(pos(), 0);
        friction_body->SetLinearVelocity(b2Vec2(v.x, v.y));
    }
    void Ambulator::ambulate_force (float f) {
        friction_joint->SetMaxForce(f);
    }
    void Ambulator::materialize () { Object::materialize(); friction_body->SetActive(true); }
    void Ambulator::dematerialize () { Object::dematerialize(); friction_body->SetActive(false); }

}


