
#ifdef HEADER
struct RBullet {
	int power;
	float mass;
	Vec pos0;
	Vec pos1;
	Vec pos2;
	Vec vel;  // Measured in blocks per FRAME not second.
	Object* owner;
	int lifetime;
	RBullet ();
	void move ();
	void draw ();
};
RBullet* fire_rbullet (Object* owner, Vec pos, Vec vel, int power = 48, float mass = 0.2);
inline RBullet* fire_rbullet_dir (Object* owner, Vec pos, float angle, float vel, int power = 48, float spread = 0.01, float mass = 0.2);
inline RBullet* fire_rbullet_to (Object* owner, Vec pos, Vec to, float vel, int power = 48, float spread = 0.01, float mass = 0.2);
#else

RBullet::RBullet () :lifetime(-2) { }

void RBullet::move () {
	if (lifetime < 0) return;
	if (lifetime == 2) owner = NULL;
	lifetime++;
	pos0 = pos2;
	pos1 = Vec(-1/0.0, -1/0.0);
	pos2 = pos0 + vel;
	LineChecker coll;
	coll = check_line(pos0, pos2, cf::bullet.maskBits, owner);
	while (coll.hit) {
		dbg(3, "Bullet hit %08x, cf %u.\n", coll.hit->GetBody()->GetUserData(), coll.hit->GetFilterData().categoryBits);
		pos1 = pos0 + coll.frac * vel;
		 // This is how you bounce.
		float velnorm = dot(vel, coll.norm);
		vel -= 2 * velnorm * coll.norm;
		coll.hit->GetBody()->ApplyLinearImpulse(mass*FPS*velnorm*coll.norm, pos1);
		 // We've ended up with a somewhat more than 100% elastic collision, but oh well.
		FixProp* fp = (FixProp*) coll.hit->GetUserData();
		if (fp->damage_factor) {
			Object* o = (Object*)coll.hit->GetBody()->GetUserData();
			if (fp == &type::rata_fixprop_helmet) {
				float angle = ang(coll.norm);
				dbg(4, "Helmet strike: %f,%f; %f > %f == %d\n", coll.norm.x, coll.norm.y, angle, rata->helmet_angle, gt_angle(angle, rata->helmet_angle));
				if (gt_angle(angle, rata->helmet_angle)) {
					snd::def[snd::helmethit2].play(dither(1.0, 0.02));
					goto just_bounce;
				}
			}
			if (o->type == type::rata) {
				if (rata->hurt_type_0 == type::bullet
				 || rata->hurt_type_1 == type::bullet) goto no_damage;
				rata->hurt_type_1 = rata->hurt_type_0;
				rata->hurt_type_0 = type::bullet;
			}
			o->damage(power * fp->damage_factor);
			no_damage: { }
			coll.hit = NULL;
			pos2 = Vec(-1/0.0, -1/0.0);
			lifetime = -1;
		}
		else {
			just_bounce:
			if (velnorm > -0.8) {
				snd::def[snd::ricochet].play(0.7+rand()*0.3/RAND_MAX, 50);
				pos2 = pos1 + (0.99-coll.frac) * vel;
				coll = check_line(pos1, pos2, cf::bullet.maskBits, (Object*)coll.hit->GetBody()->GetUserData());
			}
			else {
				coll.hit = NULL;
				pos2 = Vec(-1/0.0, -1/0.0);
				lifetime = -1;
			}
		}
	}
	vel.y -= 30/FPS/FPS;
}
void RBullet::draw () {
	if (lifetime < -1) return;
	if (pos1.x == -1/0.0) {
		window->Draw(sf::Shape::Line(
			pos0.x, pos0.y,
			pos2.x, pos2.y,
			1*PX, Color(0xffffff7f)
		));
	}
	else {
		window->Draw(sf::Shape::Line(
			pos0.x, pos0.y,
			pos1.x, pos1.y,
			1*PX, Color(0xffffff7f)
		));
		if (pos2.x != -1/0.0) {
			window->Draw(sf::Shape::Line(
				pos1.x, pos1.y,
				pos2.x, pos2.y,
				1*PX, Color(0xffffff7f)
			));
		}
		else {
			lifetime = -2;
		}
	}
}

RBullet* fire_rbullet (Object* owner, Vec pos, Vec vel, int power, float mass) {
	uint oldest = 0;
	float oldesttime = -1;
	for (uint i=0; i < MAX_BULLETS; i++) {
		if (bullets[i].lifetime < 0) {
			bullets[i].pos2 = pos;
			bullets[i].vel = (1/FPS)*vel;
			bullets[i].power = power;
			bullets[i].mass = mass;
			bullets[i].owner = owner;
			bullets[i].lifetime = 0;
			dbg(3, "Creating bullet %d\n", i);
			return &bullets[i];
		}
		else if (bullets[i].lifetime > oldesttime) {
			oldest = i;
			oldesttime = bullets[i].lifetime;
		}
	}
	bullets[oldest].pos2 = pos;
	bullets[oldest].vel = (1/FPS)*vel;
	bullets[oldest].power = power;
	bullets[oldest].mass = mass;
	bullets[oldest].owner = owner;
	bullets[oldest].lifetime = 0;
	dbg(3, "Rewriting bullet %d\n", oldest);
	return &bullets[oldest];
}
inline RBullet* fire_rbullet_dir (Object* owner, Vec pos, float angle, float vel, int power, float spread, float mass) {
	angle = dither(angle, spread);
	return fire_rbullet(owner, pos, Vec(vel*cos(angle), vel*sin(angle)), power, mass);
}
inline RBullet* fire_rbullet_to (Object* owner, Vec pos, Vec to, float vel, int power, float spread, float mass) {
	return fire_rbullet_dir(owner, pos, ang(to - pos), vel, power, spread, mass);
}

#endif

