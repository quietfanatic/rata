
#include "../inc/aux.h"

namespace phys {

    Ambulator::Ambulator (Object* obj) {
        b2BodyDef b2bdf;
        b2bdf.type = b2_kinematicBody;
        b2bdf.gravityScale = 0;
        b2bdf.fixedRotation = true;
        friction_body = space->b2world->CreateBody(&b2bdf);

        b2FrictionJointDef b2jdf;
        b2jdf.bodyA = obj->b2body;
        b2jdf.bodyB = friction_body;
        b2jdf.maxForce = 0;
        b2jdf.maxTorque = 0;
        friction_joint = (b2FrictionJoint*)space->b2world->CreateJoint(&b2jdf);
    }
    void Ambulator::ambulate_x (Object* obj, float x) {
        friction_body->SetTransform(obj->pos(), 0);
        friction_body->SetLinearVelocity(b2Vec2(x, obj->vel().y));
    }
    void Ambulator::ambulate_y (Object* obj, float y) {
        friction_body->SetTransform(obj->pos(), 0);
        friction_body->SetLinearVelocity(b2Vec2(obj->vel().x, y));
    }
    void Ambulator::ambulate_both (Object* obj, Vec v) {
        friction_body->SetTransform(obj->pos(), 0);
        friction_body->SetLinearVelocity(b2Vec2(v.x, v.y));
    }
    void Ambulator::ambulate_force (float f) {
        friction_joint->SetMaxForce(f);
    }
    void Ambulator::enable () { friction_body->SetActive(true); }
    void Ambulator::disable () { friction_body->SetActive(false); }
    Ambulator::~Ambulator () { space->b2world->DestroyBody(friction_body); }  // joint goes with

}

