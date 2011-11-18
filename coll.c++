


#define DAMAGE_KNOCKBACK 12.0
void apply_touch_damage (Object* a, Object* b, FixProp* afp, FixProp* bfp, b2Manifold* manifold) {
	if (afp == &rata_fixprop_helmet) {
		Vec norm = manifold->localNormal;
		float angle = ang(norm);
		//printf("Normal: %f, %f\n", norm.y, norm.x);
		//printf("gt_angle(%f, %f) = %d\n", rata->helmet_angle, angle, gt_angle(rata->helmet_angle, angle));
		if (gt_angle(rata->helmet_angle, angle))
			return;
	}

	if (a->life <= 0) return;
	if (a->desc->id == obj::rata) {
		if (rata->hurt_id[0] == b->desc->id
		 || rata->hurt_id[1] == b->desc->id) return;
		rata->hurt_id[1] = rata->hurt_id[0];
		rata->hurt_id[0] = b->desc->id;
	}
	else if (!bfp->damages_enemies) return;
	
	a->damage(bfp->touch_damage * afp->damage_factor);

	if (manifold->type == b2Manifold::e_faceA)
		a->impulse(manifold->localNormal*DAMAGE_KNOCKBACK);
	else if (manifold->type == b2Manifold::e_faceB)
		a->impulse(-manifold->localNormal*DAMAGE_KNOCKBACK);
}


struct myCL : b2ContactListener {
	void PreSolve (b2Contact* contact, const b2Manifold* oldmanifold) {
		if (!contact->IsTouching() || !contact->IsEnabled()) return;
		Object* a = (Object*) contact->GetFixtureA()->GetBody()->GetUserData();
		Object* b = (Object*) contact->GetFixtureB()->GetBody()->GetUserData();
		//b2Manifold* manifold = contact->GetManifold();
		if (!a->alive || !b->alive)
			return contact->SetEnabled(false);
	}
	void PostSolve (b2Contact* contact, const b2ContactImpulse* ci) {
		if (!contact->IsTouching()) return;
		Object* a = (Object*) contact->GetFixtureA()->GetBody()->GetUserData();
		FixProp* afp = (FixProp*) contact->GetFixtureA()->GetUserData();
		Object* b = (Object*) contact->GetFixtureB()->GetBody()->GetUserData();
		FixProp* bfp = (FixProp*) contact->GetFixtureB()->GetUserData();
		b2Manifold* manifold = contact->GetManifold();
		dbg(7, "COLL: %08x %08x @ (% 7.4f, % 7.4f) [% 8.4f, % 8.4f], [% 8.4f, % 8.4f]\n",
			a, b, manifold->localNormal.x, manifold->localNormal.y,
			ci->normalImpulses[0], ci->tangentImpulses[1],
			ci->normalImpulses[1], ci->tangentImpulses[1]);
		if (afp->is_standable && bfp->stands) {
			Walking* wb = (Walking*)b;
			if (manifold->type == b2Manifold::e_faceA
			 && manifold->localNormal.y > 0.7) {
				wb->floor = a;
				wb->floor_fix = contact->GetFixtureA();
				wb->floor_contact = contact;
				wb->floor_normal = manifold->localNormal;
			}
			else if (manifold->type == b2Manifold::e_faceB
				  && manifold->localNormal.y < -0.7) {
				wb->floor = a;
				wb->floor_fix = contact->GetFixtureA();
				wb->floor_contact = contact;
				wb->floor_normal = -manifold->localNormal;
			}
		}
		if (bfp->is_standable && afp->stands) {
			Walking* wa = (Walking*)a;
			if (manifold->type == b2Manifold::e_faceB
			 && manifold->localNormal.y > 0.7) {
				wa->floor = b;
				wa->floor_fix = contact->GetFixtureB();
				wa->floor_contact = contact;
				wa->floor_normal = manifold->localNormal;
			}
			else if (manifold->type == b2Manifold::e_faceA
				  && manifold->localNormal.y < -0.7) {
				wa->floor = b;
				wa->floor_fix = contact->GetFixtureB();
				wa->floor_contact = contact;
				wa->floor_normal = -manifold->localNormal;
			}
		}
		if (bfp->touch_damage && afp->damage_factor)
			apply_touch_damage(a, b, afp, bfp, manifold);
		if (afp->touch_damage && bfp->damage_factor)
			apply_touch_damage(b, a, bfp, afp, manifold);
		if (a->desc->id == obj::heart && b->desc->id == obj::rata) {
			rata->heal(48);
			a->destroy();
		}
		else if (b->desc->id == obj::heart && a->desc->id == obj::rata) {
			rata->heal(48);
			b->destroy();
		}
	}
	void EndContact (b2Contact* contact) {
		if (((FixProp*)contact->GetFixtureA()->GetUserData())->stands) {
			Walking* a = (Walking*) contact->GetFixtureA()->GetBody()->GetUserData();
			if (a->floor == (Object*)contact->GetFixtureB()->GetBody()->GetUserData()) a->floor = NULL;
		}
		if (((FixProp*)contact->GetFixtureB()->GetUserData())->stands) {
			Walking* b = (Walking*) contact->GetFixtureB()->GetBody()->GetUserData();
			if (b->floor == (Object*)contact->GetFixtureA()->GetBody()->GetUserData()) b->floor = NULL;
		}
	}
};




