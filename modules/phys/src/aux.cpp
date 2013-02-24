
#include "../inc/aux.h"

namespace phys {

    Feet::Feet (Object* obj) {
        b2BodyDef b2bdf;
        b2bdf.type = b2_kinematicBody;
        b2bdf.gravityScale = 0;
        b2bdf.fixedRotation = true;
        friction_body = space->CreateBody(&b2bdf);

        b2FrictionJointDef b2jdf;
        b2jdf.bodyA = obj->b2body;
        b2jdf.bodyB = friction_body;
        b2jdf.maxForce = 0;
        b2jdf.maxTorque = 0;
        friction_joint = (b2FrictionJoint*)space->CreateJoint(&b2jdf);
    }
    void Feet::ambulate_x (Object* obj, float x) {
        friction_body->SetTransform(obj->pos(), 0);
        friction_body->SetLinearVelocity(b2Vec2(x, obj->vel().y));
    }
    void Feet::ambulate_y (Object* obj, float y) {
        friction_body->SetTransform(obj->pos(), 0);
        friction_body->SetLinearVelocity(b2Vec2(obj->vel().x, y));
    }
    void Feet::ambulate_both (Object* obj, Vec v) {
        friction_body->SetTransform(obj->pos(), 0);
        friction_body->SetLinearVelocity(b2Vec2(v.x, v.y));
    }
    void Feet::ambulate_force (float f) {
        friction_joint->SetMaxForce(f);
    }
    void Feet::enable () { friction_body->SetActive(true); }
    void Feet::disable () { friction_body->SetActive(false); }
    Feet::~Feet () { space->DestroyBody(friction_body); }  // joint goes with


}


