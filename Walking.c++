
#ifdef HEADER

struct Walking;

#else

struct Walking : Object {
	Object*    floor;
	b2Fixture* floor_fix;
	b2Contact* floor_contact;
	Vec        floor_normal;
	b2Body*          friction_body;
	b2FrictionJoint* friction_joint;
	float floor_friction;
	float ideal_xvel;

	Walking (actor::Def* def) :
		Object(def),
		floor(NULL),
		floor_friction(0.4),
		ideal_xvel(0)
	{
		 // Create friction body
		b2BodyDef fricdef;
		fricdef.type = b2_kinematicBody;
		fricdef.position = pos;
		fricdef.active = false;
		friction_body = world->CreateBody(&fricdef);
		 // Create joint
		b2FrictionJointDef fjd;
		fjd.bodyA = body;
		fjd.bodyB = friction_body;
		fjd.localAnchorA = Vec(0, 0);
		fjd.localAnchorB = Vec(0, 0);
		fjd.maxForce = 0;
		fjd.maxTorque = 0;
		friction_joint = (b2FrictionJoint*)world->CreateJoint(&fjd);
	}

	void activate () {
		Object::activate();
		friction_body->SetActive(true);
		printf("Activated a Walking.\n");
	}
	void deactivate () {
		friction_body->SetActive(false);
		Object::deactivate();
	}

	void before_move () {
		friction_body->SetTransform(pos, 0);
		if (floor) {
			friction_body->SetActive(true);
			friction_joint->SetMaxForce(
				  body->GetMass()
				* sqrt(floor_friction * floor_fix->GetFriction())
				* 30  // Some kind of gravity
			);
			if (ideal_xvel == 0) {
				friction_body->SetLinearVelocity(b2Vec2(0, 0));
			}
			else if (abs_f(floor_normal.x) < 0.01)
				friction_body->SetLinearVelocity(b2Vec2(ideal_xvel, 0));
			else if (sign_f(floor_normal.x) == sign_f(ideal_xvel)) {
				friction_body->SetLinearVelocity(b2Vec2(
					ideal_xvel,
					-ideal_xvel * floor_normal.x / floor_normal.y
				));
			}
			else {
				friction_body->SetLinearVelocity(b2Vec2(
					ideal_xvel * floor_normal.y,
					-ideal_xvel * floor_normal.x
				));
			}
		}
		else {
			friction_joint->SetMaxForce(0);
		}

		Object::before_move();
	}

};


#endif



