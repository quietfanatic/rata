



// There are four things handled here:
//  object ids
//  object definitions
//  object descriptions
//  object instances




#ifdef DEF_ONLY

struct Object;
struct Rata;
struct Tilemap;
struct Solid;
struct Bullet;


struct Data {
	uint x;
	template <class T> Data(T x_) :x(*(uint*)&x_) { }
	template <class T> operator T () { return *(T*)&x; }
	bool operator == (Data o) { return x == o.x; }
};


// All object-related things are in the obj:: namespace except for the 
// structs representing active instances.

namespace obj {
	enum ID {
		object,
		rata,
		entrance,
		exit,
		door,
		solid,
		tilemap,
		bullet,
		rat,
		crate,
		mousehole,
		hiteffect,
		patroller,
		heart,
		item,
		clickable_text,
		tilemap_editor,
		tilepicker,
		roomsettings,
		editor_menu
	};
#ifdef MAPEDITOR
	char* idname [] = {
		"obj::object",
		"obj::rata",
		"obj::entrance",
		"obj::exit",
		"obj::door",
		"obj::solid",
		"obj::tilemap",
		"obj::bullet",
		"obj::rat",
		"obj::crate",
		"obj::mousehole",
		"obj::hiteffect",
		"obj::patroller",
		"obj::heart",
		"obj::item",
		"obj::clickable_text",
		"obj::tilemap_editor",
		"obj::tilepicker",
		"obj::editor_menu"
	};
#endif
	struct Desc;
	struct Def;
	extern const Def def [];
	template <class T> Object* ALLOC () { return new T; }
}


struct obj::Def {
	const char* name;
	uint nfixes;
	b2FixtureDef* fixdef;
	float depth;
	float order;
	Object* (& alloc) ();
	img::Image* image;
};

struct obj::Desc {
	uint16 id;
	bool temp;
	float x;
	float y;
	float xvel;
	float yvel;
	int facing;
	Data data;
	Data data2;
	Object* manifest ();

	 // Alright, we'll use a C++ constructor...just for default args.
	Desc (uint16 id_=0, void* data_=NULL, float x_=0, float y_=0, float xvel_=0, float yvel_=0, int facing_=0, bool temp_=false, uint32 data2_=0)
	 :id(id_), data(data_), x(x_), y(y_), xvel(xvel_), yvel(yvel_), facing(facing_), temp(temp_), data2(data2_)
		{ }
};

 // Fixture properties

struct FixProp {
	bool is_standable;
	bool stands;
	float damage_factor;
	int touch_damage;
	bool damages_enemies;
} default_FixProp = {true, false, 0.0, 0, false};


#else

 // Collision filters

namespace cf {
	b2Filter disabled = {0, 0, 0};
	b2Filter rata = {1, 2|4|8|16|32, 0};
	b2Filter rata_invincible = {1, 2|8|16|32, 0};
	b2Filter rata_sensor = {1, 2|32, 0};
	b2Filter solid = {2, 1|4|8|16|32, 0};
	b2Filter bullet = {4, 1|2|8|32, 0};
	b2Filter enemy = {8, 1|2|4|8|32, 0};
	b2Filter pickup = {16, 1|2|32, 0};
	b2Filter movable = {32, 1|2|4|8|16|32, 0};
	b2Filter scenery = {64, 2, 0};
};



 // INSTANCE STRUCTURES

struct Object {
	obj::Desc* desc;
	Object* next_depth;
	Object* next_order;
	b2Body* body;
	bool doomed;
	int facing;  // 1 = right, -1 = left
	uint subimage;
	Object* floor;
	b2Fixture* floor_fix;
	b2Contact* floor_contact;
	b2Vec2 floor_normal;
	int life;
	int max_life;
	

	 // Create a Box2d body for this object
	void make_body (obj::Desc* desc, bool dynamic=false, bool bullet=false) {
		b2BodyDef d;
		if (dynamic) d.type = b2_dynamicBody;
		d.position.Set(desc->x, desc->y);
		d.linearVelocity.Set(desc->xvel, desc->yvel);
		d.fixedRotation = true;
		d.userData = this;
		d.gravityScale = 1.0;
		d.bullet = bullet;
		body = world->CreateBody(&d);
	}
	
	 // Find one object (by default solid) in the area given by world coords
	 // This has false positives.
	struct AreaChecker : public b2QueryCallback {
		b2Fixture* found;
		uint16 cat;
		bool ReportFixture(b2Fixture* fix) {
			if (fix->GetFilterData().categoryBits & cat) {
				found = fix;
				return false;
			}
			else {
				return true;
			}
		}
	};
	Object* check_area (float l, float t, float r, float b, uint16 cat = 2|32) {
		AreaChecker checker;
		checker.found = NULL;
		checker.cat = cat;
		b2AABB aabb;
		if (l > r) SWAP(l, r)
		if (t > b) SWAP(t, b)
		aabb.lowerBound.Set(l, b);
		aabb.upperBound.Set(r, t);
		world->QueryAABB(&checker, aabb);
		if (checker.found)
			return (Object*) checker.found->GetBody()->GetUserData();
		else return NULL;
	}

	 // Find one object (by default solid) along the line in world coords
	struct LineChecker : public b2RayCastCallback {
		b2Fixture* hit;
		float dist;
		uint16 cat;
		float32 ReportFixture(b2Fixture* fix, const b2Vec2& p, const b2Vec2& n, float32 f) {
			if (fix->GetFilterData().categoryBits & cat) {
				if (f < dist) {
					hit = fix;
					dist = f;
				}
				return dist;
			}
			return 1;
		}
	};

	Object* check_line (float x1, float y1, float x2, float y2, uint16 cat = cf::solid.categoryBits) {
		LineChecker checker;
		checker.hit = NULL;
		checker.cat = cat;
		checker.dist = 1;
		world->RayCast(&checker, b2Vec2(x1, y1), b2Vec2(x2, y2));
		if (checker.hit) {
			return (Object*)checker.hit->GetBody()->GetUserData();
		}
		else return NULL;
	}
	 // Get contacts of a fixture
	Object* get_touching (b2Fixture* fix) {
		Object* r = NULL;
		for (b2ContactEdge* ce = body->GetContactList(); ce; ce = ce->next) {
			b2Contact* c = ce->contact;
			if (c->IsEnabled() && c->IsTouching()) {
				if (c->GetFixtureA() == fix)
					r = (Object*)c->GetFixtureB()->GetBody()->GetUserData();
				else if (c->GetFixtureB() == fix)
					r = (Object*)c->GetFixtureA()->GetBody()->GetUserData();
			}
		}
		return r;
	}
	Object* get_floor (b2Fixture* fix) {
		Object* r = NULL;
		for (b2ContactEdge* ce = body->GetContactList(); ce; ce = ce->next) {
			b2Contact* c = ce->contact;
			if (c->IsEnabled() && c->IsTouching()) {
				if (c->GetFixtureA() == fix) {
					Object* other = (Object*)c->GetFixtureB()->GetBody()->GetUserData();
					if (yvelrel(other) <= 0.4)
						r = other;
				}
				else if (c->GetFixtureB() == fix) {
					Object* other = (Object*)c->GetFixtureA()->GetBody()->GetUserData();
					if (yvelrel(other) <= 0.4)
						r = other;
				}
			}
		}
		return r;
	}

	 // Events
	virtual void on_create () {
		if (obj::def[desc->id].nfixes) {
			make_body(desc, true);
			for (uint i = obj::def[desc->id].nfixes; i > 0; i--) {
				dbg(4, "Fix %d: 0x%08x\n", i, body->CreateFixture(&(obj::def[desc->id].fixdef[i-1])));
			}
			dbg(3, "Affixed 0x%08x with 0x%08x\n", this, body);
		}
		else body = NULL;
		facing = desc->facing;
		floor = NULL;
		floor_contact = NULL;
		life = max_life = 0;
		subimage = 0;
//		realtest = 151783;
	}
	virtual void before_move () { }
	virtual void after_move () { }
	virtual void on_destroy () { }
	virtual void draw () {
		if (def()->image) {
			if (body) {
				draw_image(def()->image, x(), y(), subimage, facing == 1);
			}
			else {
				draw_image(def()->image, desc->x, desc->y, subimage, facing == 1);
			}
		}
	}

	virtual void damage (int d) { life -= d; if (life <= 0) kill(); }
	virtual void heal (int h) { life += h; if (life > max_life) life = max_life; }
	virtual void kill () { destroy(); }
	virtual char* describe () { return "What a mysterious object."; }

	 // Non-overridable
	void create () {
		next_depth = creation_queue;
		creation_queue = this;
		next_order = NULL;
		doomed = false;
		dbg(2, "Creating 0x%08x\n", this);
	}
	void destroy () {
		doomed = true;
		dbg(2, "Destroying 0x%08x\n", this);
	}

	 // Wrap b2Body
	float x() { return body->GetPosition().x; }
	float y() { return body->GetPosition().y; }
	float xvel() { return body->GetLinearVelocity().x; }
	float yvel() { return body->GetLinearVelocity().y; }
	float speed() { return xvel()*xvel() + yvel()*yvel(); }
	float mass() { return body->GetMass(); }
	float xvelrel(Object* other) {
		return xvel() - other->xvel();
	}
	float yvelrel(Object* other) {
		return yvel() - other->yvel();
	}
	void set_pos(float x, float y) {
		body->SetTransform(b2Vec2(x, y), 0);
	}
	void set_vel(float x, float y) {
		body->SetLinearVelocity(b2Vec2(x, y));
	}
	void add_vel(float x, float y) {
		b2Vec2 oldvel = body->GetLinearVelocity();
		body->SetLinearVelocity(b2Vec2(oldvel.x+x, oldvel.y+y));
	}
	void impulse (float x, float y) {
		body->ApplyLinearImpulse(b2Vec2(x, y), body->GetPosition());
	}
	void mutual_impulse(Object* other, float x, float y) {
		float m = body->GetMass();
		float om = other->body->GetMass();
		if (m == 0 || m == 1/0.0)
			other->body->ApplyLinearImpulse(b2Vec2(x, y), other->body->GetPosition());
		if (om == 0 || om == 1/0.0)
			body->ApplyLinearImpulse(b2Vec2(-x, -y), body->GetPosition());
		else {
			other->body->ApplyLinearImpulse(b2Vec2(x*m/(m+om), y*m/(m+om)), other->body->GetPosition());
			body->ApplyLinearImpulse(b2Vec2(-x*om/(m+om), -y*om/(m+om)), other->body->GetPosition());
		}
	}

	Bullet* fire_bullet(float bx, float by, float bd, float bv = 120, int power = 48, float bs=0.01) {
		bd = dither(bd, bs);
		return (Bullet*)(new obj::Desc(
			obj::bullet, this,
			bx, by, bv*cos(bd), bv*sin(bd), 1, true
		))->manifest();
	}
	Bullet* fire_bullet_to(float bx, float by, float tx, float ty, float bv = 120, int power = 48, float bs=0.01) {
		return fire_bullet(bx, by, atan2(ty-by, tx-bx), bv, power, bs);
	}

	 // Get def, id
	const uint16 id () { return desc->id; }
	const obj::Def* def () { return &obj::def[id()]; }

};

struct Solid : Object {
	virtual void on_create () {
		Object::on_create();
		make_body(desc, false, false);
		body->CreateFixture((b2FixtureDef*)desc->data);
		dbg(3, "Affixed 0x%08x with 0x%08x\n", this, body);
	}
};





 // Base class for walking objects
 //  (They need a moving friction joint)
struct Walking : Object {
	b2Body* friction_body;
	b2FrictionJoint* friction_joint;
	float floor_friction;
	float ideal_xvel;
	virtual void on_create () {
		 // Create friction body
		b2BodyDef fricdef;
		fricdef.type = b2_kinematicBody;
		fricdef.position.Set(desc->x, desc->y);
		friction_body = world->CreateBody(&fricdef);
		 // Create joint
		b2FrictionJointDef fjd;
		fjd.bodyA = body;
		fjd.bodyB = friction_body;
		fjd.localAnchorA = b2Vec2(0, 0);
		fjd.localAnchorB = b2Vec2(0, 0);
		fjd.maxForce = 0;
		fjd.maxTorque = 0;
		friction_joint = (b2FrictionJoint*)world->CreateJoint(&fjd);
		 // Some default values
		floor_friction = 0.4;
	}
	virtual void before_move () {
		friction_body->SetTransform(body->GetPosition(), 0);
		if (floor) {
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
	}
	virtual void on_destroy () {
		Object::on_destroy();
		world->DestroyBody(friction_body);
	}
};


struct Item : Object {
	void draw ();
	void before_move ();
};


#include "rata.c++"

void Item::draw () {
	if (desc->data) {
		item::Equip* info = (item::Equip*)desc->data;
		draw_image(info->appearance, desc->x, desc->y, info->world_frame);
	}
}
void Item::before_move () {
	if (rata->floor)
		rata->propose_action(Rata::action_equip, this, desc->x, desc->y, 1);
}


struct Entrance : Object {
	void on_create () {
		if (room::entrance == (int)desc->data2) {
			if (rata->body) {
				rata->set_pos(desc->x, desc->y-0.1);
				if (desc->data) rata->state = (Rata::State)(uint)desc->data;
				if (desc->facing) rata->facing = desc->facing;
			}
			else {
				rata->desc->x = desc->x;
				rata->desc->y = desc->y-0.1;
				if (desc->data) rata->desc->data = desc->data;
				if (desc->facing) rata->desc->facing = desc->facing;
			}
		}
		destroy();
	}
};

struct Exit : Object {
	void after_move () {
		if (rata->x() > desc->x)
		if (rata->x() < desc->x + desc->xvel)
		if (rata->y() > desc->y)
		if (rata->y() < desc->y + desc->yvel)
			((Room*)desc->data)->enter(desc->data2);
	}
};

struct Door : Object {
	void before_move () {
		if (rata->floor)
			rata->propose_action(Rata::action_enter, this, desc->x, desc->y, desc->xvel);
	}
};


struct Tilemap : Object {
	void on_create () { };
	bool is_standable () { return true; }
};



struct Bullet : Object {
	float oldx;
	float oldy;
	float midx;
	float midy;
	float oldxvel;
	float oldyvel;
	int lifetime;

	void on_create () {
		make_body(desc, true, true);
		for (uint i = obj::def[desc->id].nfixes; i > 0; i--) {
			dbg(4, "Fix %d: 0x%08x\n", i, body->CreateFixture(&(obj::def[desc->id].fixdef[i-1])));
		}
		dbg(3, "Affixed 0x%08x with 0x%08x\n", this, body);
		oldx = x();
		oldy = y();
		midx = -1/0.0;
		midy = -1/0.0;
		lifetime = 0;
	}

	void before_move () {
		oldx = x();
		oldy = y();
		midx = -1/0.0;
		midy = -1/0.0;
		oldxvel = xvel();
		oldyvel = yvel();
		lifetime++;
	}

	void find_hit (b2Fixture* other) {
		b2RayCastOutput out;
		b2RayCastInput in;
		in.p1 = b2Vec2(oldx, oldy);
		in.p2 = b2Vec2(oldx+oldxvel, oldy+oldyvel);
		in.maxFraction = 1.0;

		bool hit = other->RayCast(
			&out,
			in,
			0	
		);
		dbg(3, "hit: %d\n", hit);
		if (hit) {
			midx = oldx + out.fraction * oldxvel;
			midy = oldy + out.fraction * oldyvel;
		}
		else { midx = x(); midy = y(); }
	}

	void draw () {
		if (midx == -1/0.0) {
			window->Draw(sf::Shape::Line(
				oldx, oldy,
				x(), y(),
				1*PX, sf::Color(255, 255, 255, 127)
			));
		}
		else {
			window->Draw(sf::Shape::Line(
				oldx, oldy,
				midx, midy,
				1*PX, sf::Color(255, 255, 255, 127)
			));
			if (lifetime != -1) {
				window->Draw(sf::Shape::Line(
					midx, midy,
					x(), y(),
					1*PX, sf::Color(255, 255, 255, 127)
				));
			}
		}

		if (lifetime == -1) {
			destroy();
		}
	}

	void destroy_after_draw () {
		lifetime = -1;
	}
};



struct Crate : Object {
	virtual char* describe () { return "There's a wooden crate sitting here.\nIt looks like it can be pushed around."; }
	virtual void on_create () { Object::on_create(); life = max_life = 144; }
	virtual void damage (int d) { Object::damage(d); snd::woodhit.play(); }
};

struct Mousehole : Object {
	int timer;
	virtual char* describe () { return "A large metal pipe is coming out of the ground.\nFor ventilation, perhaps?\nIt's big enough for a rat to crawl through.\nMaybe that's where they're coming from."; }
	virtual void draw () {
		draw_image(&img::mousehole, desc->x, desc->y);
	}
	virtual void before_move () {
		timer--;
		if (timer <= 0) {
			timer = 300 + rand() % 1200;
			if (rata && abs_f(rata->x() - desc->x) < 3.0 && abs_f(rata->y() - desc->y)) {
				return;
			}
			(new obj::Desc (obj::rat, NULL, desc->x, desc->y, 0, 0, 0, true))->manifest();
		}
	}
	virtual void on_create () {
		make_body(desc, false, false);
		body->CreateFixture(&def()->fixdef[0]);
		timer = 300 + rand() % 1200;
	}
};

struct HitEffect : Object {
	int timer;
	uint numsubs;
	uint fpsub;
	virtual void on_create () {
		Object::on_create();
		timer = desc->xvel;
		//img::Image* image = ((img::Image*)desc->data);
		img::Image* image = &img::hit_damagable;
		numsubs = image->numsubs();
		fpsub = (timer+numsubs-1) / numsubs;
	}
	virtual void draw () {
		//img::Image* image = ((img::Image*)desc->data);
		img::Image* image = &img::hit_damagable;
		uint sub = numsubs - timer / numsubs - 1;
		draw_image(image, desc->x, desc->y, sub);
		timer--;
		if (timer == 0) destroy();
	}
};

struct Heart : Object {
	virtual char* describe () { return "Just as rats live off the refuse of humans,\nYou too can live off of the rats.\nPick this up to restore one heart."; }
};


#include "enemies.c++"
#ifdef MAPEDITOR
#include "editor_objects.c++"
#else
typedef Object ClickableText;
typedef Object TilemapEditor;
typedef Object TilePicker;
typedef Object RoomSettings;
typedef Object EditorMenu;
#endif

 // loose end from above

Object* obj::Desc::manifest () {
	Object* r = obj::def[id].alloc();
	r->desc = this;
	r->body = NULL;
	r->create();
	return r;
}


 // Some utilities for this file


b2PolygonShape* make_poly (uint n, b2Vec2* vs, float radius = 0.01) {
	b2PolygonShape* r = new b2PolygonShape;
	r->Set(vs, n);
	r->m_radius = radius;
	return r;
}

b2PolygonShape* make_rect (float w, float h, float radius = 0.01) {
	return make_poly(4, (b2Vec2[]){
		b2Vec2(-w/2, 0),
		b2Vec2(w/2, 0),
		b2Vec2(w/2, h),
		b2Vec2(-w/2, h)
	}, radius);
}

b2CircleShape* make_circle (float rad, float x = 0, float y = 0) {
	b2CircleShape* r = new b2CircleShape;
	r->m_radius = rad;
	r->m_p = b2Vec2(x, y);
	return r;
}


inline b2FixtureDef make_fixdef (b2Shape* shape, b2Filter filter, float friction=0, float restitution=0, float density=0, FixProp* prop=NULL, bool sensor=false) {
	b2FixtureDef r;
	r.shape = shape;
	r.friction = friction;
	r.restitution = restitution;
	r.density = density;
	r.filter = filter;
	r.userData = prop?prop:&default_FixProp;
	r.isSensor = sensor;
	return r;
}

 // OBJECT DEFINITIONS


b2Vec2 rata_poly_feet [] = {
	b2Vec2(-2.5*PX, 0*PX),
	b2Vec2(2.5*PX, 0*PX),
	b2Vec2(2.5*PX, 1*PX),
	b2Vec2(-2.5*PX, 1*PX)
};
b2Vec2 rata_poly_27 [] = {
	b2Vec2(-2.5*PX, 0.1*PX),
	b2Vec2(2.5*PX, 0.1*PX),
	b2Vec2(5.5*PX, 17*PX),
	b2Vec2(5.5*PX, 24*PX),
	b2Vec2(0*PX, 27*PX),
	b2Vec2(-5.5*PX, 24*PX),
	b2Vec2(-5.5*PX, 17*PX)
};
b2Vec2 rata_poly_25 [] = {
	b2Vec2(-2.5*PX, 0.1*PX),
	b2Vec2(2.5*PX, 0.1*PX),
	b2Vec2(5.5*PX, 15*PX),
	b2Vec2(5.5*PX, 22*PX),
	b2Vec2(0*PX, 25*PX),
	b2Vec2(-5.5*PX, 22*PX),
	b2Vec2(-5.5*PX, 15*PX)
};
b2Vec2 rata_poly_21 [] = {
	b2Vec2(-2.5*PX, 0.1*PX),
	b2Vec2(2.5*PX, 0.1*PX),
	b2Vec2(5.5*PX, 11*PX),
	b2Vec2(5.5*PX, 18*PX),
	b2Vec2(0*PX, 21*PX),
	b2Vec2(-5.5*PX, 18*PX),
	b2Vec2(-5.5*PX, 11*PX)
};
b2Vec2 rata_poly_h7 [] = {
	b2Vec2(-8*PX, 0.1*PX),
	b2Vec2(8*PX, 0.1*PX),
	b2Vec2(4*PX, 7*PX),
	b2Vec2(-4*PX, 7*PX),
};

FixProp rata_fixprop = {true, false, 1.0, 0, false};
FixProp rata_fixprop_feet = {false, true, 1.0, 0, false};
FixProp rata_fixprop_helmet = {true, true, 1.0, 0, false};
b2FixtureDef rata_fixes [] = {
	make_fixdef(make_poly(4, rata_poly_feet), cf::rata, 0, 0, 1.0, &rata_fixprop_feet, false),
	make_fixdef(make_poly(7, rata_poly_27), cf::rata, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(7, rata_poly_25), cf::rata_sensor, 0, 0, 1.0, &rata_fixprop, true),
	make_fixdef(make_poly(7, rata_poly_21), cf::rata_sensor, 0, 0, 1.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_h7), cf::rata_sensor, 0, 0, 1.0, &rata_fixprop, true),
	make_fixdef(make_circle(7.5*PX, 0*PX, 23.5*PX), cf::rata, 0, 0, 0.0, &rata_fixprop_helmet, false),
	make_fixdef(make_circle(7.5*PX, 0*PX, 16.5*PX), cf::rata, 0, 0, 0.0, &rata_fixprop_helmet, false),
	make_fixdef(make_circle(7.5*PX, 4.5*PX, 11.5*PX), cf::rata, 0, 0, 0.0, &rata_fixprop_helmet, false),
	make_fixdef(make_circle(7.5*PX, -4.5*PX, 11.5*PX), cf::rata, 0, 0, 0.0, &rata_fixprop_helmet, false)
};

FixProp bullet_fixprop = {false, false, 0.0, 48, true};
b2FixtureDef bullet_fix = make_fixdef(make_circle(0.05), cf::bullet, 0, 0.8, 10.0, &bullet_fixprop);
FixProp rat_fixprop = {true, true, 1.0, 12, false};
b2FixtureDef rat_fix = make_fixdef(make_rect(12*PX, 5*PX), cf::enemy, 0, 0, 4.0, &rat_fixprop);
FixProp crate_fixprop = {true, false, 1.0, 0, false};
b2FixtureDef crate_fix = make_fixdef(make_rect(1, 1, 0.005), cf::movable, 0.4, 0, 4.0, &crate_fixprop);
b2FixtureDef mousehole_fix = make_fixdef(make_rect(14*PX, 10*PX), cf::scenery, 0, 0, 0);
FixProp patroller_fixprop = {true, true, 1.0, 0, false};
b2FixtureDef patroller_fixes [] = {
	make_fixdef(make_rect(14*PX, 12*PX), cf::enemy, 0, 0.1, 6.0, &patroller_fixprop),
};
FixProp heart_fixprop = {false, false, 0.0, 0, false};
b2FixtureDef heart_fix = make_fixdef(make_rect(0.5, 0.5), cf::pickup, 0.8, 0, 0.1, &heart_fixprop);

const obj::Def obj::def [] = {

	{"Object", 0, NULL, 0, 0, obj::ALLOC<Object>, NULL},
	{"Rata", 9, rata_fixes, 10, 100, obj::ALLOC<Rata>, NULL},
	{"Entrance", 0, NULL, -1000, -1000, obj::ALLOC<Entrance>, NULL},
	{"Exit", 0, NULL, -100, -100, obj::ALLOC<Exit>, NULL},
	{"Door", 0, NULL, -100, 200, obj::ALLOC<Door>, NULL},
	{"Solid Object", 0, NULL, 0, 0, obj::ALLOC<Solid>, NULL},
	{"Tilemap", 0, NULL, 0, 0, obj::ALLOC<Tilemap>, NULL},
	{"Bullet", 1, &bullet_fix, -10, 50, obj::ALLOC<Bullet>, NULL},
	{"Rat", 1, &rat_fix, 15, 10, obj::ALLOC<Rat>, &img::rat},
	{"Crate", 1, &crate_fix, 0, 0, obj::ALLOC<Crate>, &img::crate},
	{"Mousehole", 1, &mousehole_fix, 50, 0, obj::ALLOC<Mousehole>, &img::mousehole},
	{"Hit Effect", 0, NULL, -90, 0, obj::ALLOC<HitEffect>, NULL},
	{"Patroller", 1, patroller_fixes, 20, 20, obj::ALLOC<Patroller>, &img::patroller},
	{"Heart", 1, &heart_fix, -20, 0, obj::ALLOC<Heart>, &img::heart},
	{"Item", 0, NULL, -5, 150, obj::ALLOC<Item>, NULL},
	{"Clickable text", 0, NULL, -2000, 2000, obj::ALLOC<ClickableText>, NULL},
	{"Tilemap editor", 0, NULL, -100, 100, obj::ALLOC<TilemapEditor>, NULL},
	{"Tile picker", 0, NULL, -1000, 1000, obj::ALLOC<TilePicker>, NULL},
	{"Rooms Settings pane", 0, NULL, -1000, 1000, obj::ALLOC<RoomSettings>, NULL},
	{"Editor menu", 0, NULL, -1900, 1900, obj::ALLOC<EditorMenu>, NULL}

};



#define DAMAGE_KNOCKBACK 12.0
void apply_touch_damage (Object* a, Object* b, FixProp* afp, FixProp* bfp, b2Manifold* manifold) {
	if (afp == &rata_fixprop_helmet) {
		b2Vec2 norm = manifold->localNormal;
		float angle = atan2(norm.y, norm.x);
		printf("Normal: %f, %f\n", norm.y, norm.x);
		printf("gt_angle(%f, %f) = %d\n", rata->helmet_angle, angle, gt_angle(rata->helmet_angle, angle));
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

	if (b->desc->id == obj::bullet) {
		(new obj::Desc(
			obj::hiteffect, &img::hit_damagable,
			((Bullet*)b)->midx, ((Bullet*)b)->midy, 11, 0, 0, true
		))->manifest();
		((Bullet*)b)->destroy_after_draw();
	}
	else if (manifold->type == b2Manifold::e_faceA)
		a->impulse(manifold->localNormal.x*DAMAGE_KNOCKBACK,
		           manifold->localNormal.y*DAMAGE_KNOCKBACK);
	else if (manifold->type == b2Manifold::e_faceB)
		a->impulse(-manifold->localNormal.x*DAMAGE_KNOCKBACK,
		           -manifold->localNormal.y*DAMAGE_KNOCKBACK);
}


struct myCL : b2ContactListener {
	void PreSolve (b2Contact* contact, const b2Manifold* oldmanifold) {
		if (!contact->IsTouching()) return;
		Object* a = (Object*) contact->GetFixtureA()->GetBody()->GetUserData();
		Object* b = (Object*) contact->GetFixtureB()->GetBody()->GetUserData();
		//b2Manifold* manifold = contact->GetManifold();
		if (a->doomed or b->doomed)
			return contact->SetEnabled(false);
		if (a->desc->id == obj::bullet) {
			Bullet* ba = (Bullet*) a;
			if (ba->lifetime == -1
			 || (ba->lifetime < 2 && ba->desc->data == b)) {
				return contact->SetEnabled(false);
			}
		}
		if (b->desc->id == obj::bullet) {
			Bullet* bb = (Bullet*) b;
			if (bb->lifetime == -1
			 || (bb->lifetime < 2 && bb->desc->data == a)) {
				return contact->SetEnabled(false);
			}
		}
	}
	void PostSolve (b2Contact* contact, const b2ContactImpulse* ci) {
		//if (!contact->IsTouching()) return;
		Object* a = (Object*) contact->GetFixtureA()->GetBody()->GetUserData();
		FixProp* afp = (FixProp*) contact->GetFixtureA()->GetUserData();
		Object* b = (Object*) contact->GetFixtureB()->GetBody()->GetUserData();
		FixProp* bfp = (FixProp*) contact->GetFixtureB()->GetUserData();
		b2Manifold* manifold = contact->GetManifold();
		if (afp->is_standable && bfp->stands) {
			if (manifold->type == b2Manifold::e_faceA
			 && manifold->localNormal.y > 0.7) {
				b->floor = a;
				b->floor_fix = contact->GetFixtureA();
				b->floor_contact = contact;
				b->floor_normal = manifold->localNormal;
			}
			else if (manifold->type == b2Manifold::e_faceB
				  && manifold->localNormal.y < -0.7) {
				b->floor = a;
				b->floor_fix = contact->GetFixtureA();
				b->floor_contact = contact;
				b->floor_normal = -manifold->localNormal;
			}
		}
		if (bfp->is_standable && afp->stands) {
			if (manifold->type == b2Manifold::e_faceB
			 && manifold->localNormal.y > 0.7) {
				a->floor = b;
				a->floor_fix = contact->GetFixtureB();
				a->floor_contact = contact;
				a->floor_normal = manifold->localNormal;
			}
			else if (manifold->type == b2Manifold::e_faceA
				  && manifold->localNormal.y < -0.7) {
				a->floor = b;
				a->floor_fix = contact->GetFixtureB();
				a->floor_contact = contact;
				a->floor_normal = -manifold->localNormal;
			}
		}
		if (a->desc->id == obj::bullet) {
			Bullet* ba = (Bullet*) a;
			ba->find_hit(contact->GetFixtureB());
			if (ci->normalImpulses[0] > 10.0
			      || ci->normalImpulses[0] < -10.0) ba->destroy_after_draw();
			else if (ba->speed() < 100) ba->destroy_after_draw();
			else snd::ricochet.play(0.7+rand()*0.3/RAND_MAX, 50);
		}
		if (b->desc->id == obj::bullet) {
			Bullet* bb = (Bullet*) b;
			bb->find_hit(contact->GetFixtureA());
			if (ci->normalImpulses[0] > 10.0
			      || ci->normalImpulses[0] < -10.0) bb->destroy_after_draw();
			else if (bb->speed() < 100) bb->destroy_after_draw();
			else snd::ricochet.play(0.7+rand()*0.3/RAND_MAX, 50);
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
		Object* a = (Object*) contact->GetFixtureA()->GetBody()->GetUserData();
		Object* b = (Object*) contact->GetFixtureB()->GetBody()->GetUserData();
		if (a->floor == b) a->floor = NULL;
		if (b->floor == a) b->floor = NULL;
	}
};







#endif

