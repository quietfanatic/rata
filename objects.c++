



// There are four things handled here:
//  object ids
//  object definitions
//  object descriptions
//  object instances




#ifdef HEADER

struct Actor;
struct Game;
struct Object;
struct Rata;
struct Tilemap;
struct Solid;
struct Bullet;



// All object-related things are in the obj:: namespace except for the 
// structs representing active instances.

namespace obj {
	enum {
		game,
		rata,
		room,
		bullet,
		back_tiles,
		bullet_layer,
		front_tiles,
		shade,
		n_types
	};
	struct Desc;
	struct Type;
	extern const uint n_globals;
	extern Desc saved [];
	extern Actor* global [];
	extern const Type type [n_types];
	template <class T> Actor* ALLOC (obj::Desc* desc) { return new T (desc); }
}


struct obj::Type {
	const char* name;
	int nfixes;
	b2FixtureDef* fixdef;
	float depth;
	Actor* (& alloc )(obj::Desc*);
	int16 image;
};

struct obj::Desc {
	int16 loc;
	int16 type;
	Vec pos;
	Vec vel;
	int32 facing;
	uint32 data;
	uint32 data2;

	Actor* manifest ();
};

 // Most "things" in the game world are this.
struct Actor : obj::Desc {
	int16 id;
	bool active;
	Actor* contents;
	Actor* next;
	Actor* prev;
	Actor* next_active;

	virtual void activate ();
	virtual void deactivate ();

	virtual void before_move () { }
	virtual void after_move () { }
	virtual void draw () { }

	Actor (obj::Desc* desc) :
		obj::Desc(*desc),
		id( desc - obj::saved < obj::n_globals ? desc - obj::saved : -1 ),
		active(false),
		contents(NULL),
		next(NULL),
		prev(NULL),
		next_active(NULL)
	{ }
	
	bool has_body () {
		return obj::type[type].nfixes > -1;
	}

	 // debug
	virtual void debug_print () {
		printf("%08x %12s: (% 8.4f, % 8.4f) @ (% 8.4f, % 8.4f) % d; %d %d\n",
			this, obj::type[id].name,
			pos.x, pos.y,
			vel.x, vel.y,
			facing, data, data2
		);
	}

	virtual void take (Actor* a) {
		if (contents) contents->prev = a;
		a->next = contents;
		contents = a;
	}
	virtual void give (Actor* a) {
		if (a == contents) {
			contents = a->next;
		}
		if (a->next) a->next->prev = a->prev;
		if (a->prev) a->prev->next = a->next;
	}
};

#else

void Actor::activate () {
	next_active = activation_queue;
	activation_queue = this;
	active = true;
	dbg(2, "Activating 0x%08x\n", this);
}
void Actor::deactivate () {
	active = false;
	dbg(2, "Deactivating 0x%08x\n", this);
}


 // INSTANCE STRUCTURES

struct Object : Actor {
	b2Body* body;
	uint subimage;
	int life;
	int max_life;
	

	 // Create a Box2d body for this object
	b2Body* make_body (b2BodyType btype = b2_staticBody, bool bullet=false) {
		b2BodyDef d;
		d.type = btype;
		d.position = pos;
		d.linearVelocity = vel;
		d.fixedRotation = true;
		d.userData = this;
		d.gravityScale = 1.0;
		d.bullet = bullet;
		d.active = false;
		return world->CreateBody(&d);
	}

	Object (obj::Desc* desc) :
		Actor(desc),
		body(obj::type[id].nfixes > 0 ? make_body(b2_dynamicBody, true) : NULL),
		life(0),
		max_life(0),
		subimage(0)
	{
		for (int i = obj::type[type].nfixes; i > 0; i--) {
			dbg(4, "Fix %d: 0x%08x\n", i, body->CreateFixture(&(obj::type[type].fixdef[i-1])));
		}
		dbg(3, "Affixed 0x%08x with 0x%08x\n", this, body);
	}
	
	 // Events
	void before_move () {
		//body->SetPosition(pos);
		body->SetLinearVelocity(vel);
	}
	void after_move () {
		pos = body->GetPosition();
		vel = body->GetLinearVelocity();
	}

	void draw () {
		if (obj::type[type].image) {
			draw_image(obj::type[type].image, pos, subimage, facing == 1);
		}
	}

	 // New things specific to object.
	virtual void damage (int d) { life -= d; if (life <= 0) kill(); }
	virtual void heal (int h) { life += h; if (life > max_life) life = max_life; }
	virtual void kill () { type = -1; deactivate(); }
	virtual char* describe () { return "What a mysterious object."; }


	float mass() { return body->GetMass(); }
	void impulse (Vec i) {
		vel += i / mass();
	}
	void mutual_impulse(Object* other, Vec i) {
		float m = mass();
		float om = other->mass();
		if (m == 0 || m == 1/0.0)
			other->impulse(i);
		if (om == 0 || om == 1/0.0)
			impulse(-i);
		else {
			other->impulse(i * m/(m+om));
			impulse(-i * om/(m+om));
		}
	}

	void activate () {
		Actor::activate();
		body->SetActive(true);
	}
	void deactivate () {
		body->SetActive(false);
		Actor::deactivate();
	}

};




 // Base class for walking objects
 //  (They need a moving friction joint)
struct Walking : Object {
	Object* floor;
	b2Fixture* floor_fix;
	b2Contact* floor_contact;
	Vec floor_normal;
	b2Body* friction_body;
	b2FrictionJoint* friction_joint;
	float floor_friction;
	float ideal_xvel;
	Walking (obj::Desc* desc) :
		Object(desc),
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
	void activate () {
		Object::activate();
		friction_body->SetActive(true);
		printf("Activated a Walking.\n");
	}
	void deactivate () {
		friction_body->SetActive(false);
		Object::deactivate();
	}
};


#include "game.c++"

struct Item : Actor {
	void draw ();
	void after_move ();
};


#include "rata.c++"

void Item::draw () {
	if (data) {
		item::Def* info = &item::def[data];
		draw_image(info->appearance, pos, info->world_frame);
	}
}
void Item::after_move () {
	if (rata->floor)
		rata->propose_action(Rata::action_equip, this, pos, 1);
}


//#include "enemies.c++"

#include "room.c++"
#include "misc.c++"


 // loose end from above

Actor* obj::Desc::manifest () {
	return obj::type[type].alloc(this);
}


 // Some utilities for this file


b2PolygonShape* make_poly (uint n, Vec* vs, float radius = 0.01) {
	b2PolygonShape* r = new b2PolygonShape;
	r->Set((b2Vec2*)vs, n);
	r->m_radius = radius;
	return r;
}

b2PolygonShape* make_rect (float w, float h, float radius = 0.01) {
	return make_poly(4, (Vec[]){
		Vec(-w/2, 0),
		Vec(w/2, 0),
		Vec(w/2, h),
		Vec(-w/2, h)
	}, radius);
}

b2CircleShape* make_circle (float rad, float x = 0, float y = 0) {
	b2CircleShape* r = new b2CircleShape;
	r->m_radius = rad;
	r->m_p = Vec(x, y);
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


Vec rata_poly_feet [] = {
	Vec(-2.5*PX, 0*PX),
	Vec(2.5*PX, 0*PX),
	Vec(2.5*PX, 1*PX),
	Vec(-2.5*PX, 1*PX)
};
Vec rata_poly_27 [] = {
	Vec(-2.5*PX, 0.1*PX),
	Vec(2.5*PX, 0.1*PX),
	Vec(5.5*PX, 17*PX),
	Vec(5.5*PX, 24*PX),
	Vec(0*PX, 27*PX),
	Vec(-5.5*PX, 24*PX),
	Vec(-5.5*PX, 17*PX)
};
Vec rata_poly_25 [] = {
	Vec(-2.5*PX, 0.1*PX),
	Vec(2.5*PX, 0.1*PX),
	Vec(5.5*PX, 15*PX),
	Vec(5.5*PX, 22*PX),
	Vec(0*PX, 25*PX),
	Vec(-5.5*PX, 22*PX),
	Vec(-5.5*PX, 15*PX)
};
Vec rata_poly_21 [] = {
	Vec(-2.5*PX, 0.1*PX),
	Vec(2.5*PX, 0.1*PX),
	Vec(5.5*PX, 11*PX),
	Vec(5.5*PX, 18*PX),
	Vec(0*PX, 21*PX),
	Vec(-5.5*PX, 18*PX),
	Vec(-5.5*PX, 11*PX)
};
Vec rata_poly_h7 [] = {
	Vec(-8*PX, 0.1*PX),
	Vec(8*PX, 0.1*PX),
	Vec(4*PX, 7*PX),
	Vec(-4*PX, 7*PX),
};
Vec rata_poly_crawl_r [] = {
	Vec(-11*PX, 0.1*PX),
	Vec(6*PX, 0.1*PX),
	Vec(10*PX, 4*PX),
	Vec(10*PX, 11*PX),
	Vec(4*PX, 14*PX),
};
Vec rata_poly_crawl_l [] = {
	Vec(-4*PX, 14*PX),
	Vec(-10*PX, 11*PX),
	Vec(-10*PX, 4*PX),
	Vec(-6*PX, 0.1*PX),
	Vec(11*PX, 0.1*PX),
};
Vec rata_poly_sensor_21 [] = {
	Vec(-2.5*PX, 0.1*PX),
	Vec(2.5*PX, 0.1*PX),
	Vec(2.5*PX, 21*PX),
	Vec(-2.5*PX, 21*PX),
};
Vec rata_poly_sensor_floor_r [] = {
	Vec(4*PX, -4*PX),
	Vec(12*PX, -4*PX),
	Vec(12*PX, 4*PX),
	Vec(4*PX, 4*PX),
};
Vec rata_poly_sensor_floor_l [] = {
	Vec(-4*PX, 4*PX),
	Vec(-12*PX, 4*PX),
	Vec(-12*PX, -4*PX),
	Vec(-4*PX, -4*PX),
};
Vec rata_poly_sensor_block_r [] = {
	Vec(4*PX, 10*PX),
	Vec(12*PX, 10*PX),
	Vec(12*PX, 12*PX),
	Vec(4*PX, 12*PX),
};
Vec rata_poly_sensor_block_l [] = {
	Vec(-4*PX, 12*PX),
	Vec(-12*PX, 12*PX),
	Vec(-12*PX, 10*PX),
	Vec(-4*PX, 10*PX),
};
Vec rata_poly_sensor_wall_r [] = {
	Vec(4*PX, 17*PX),
	Vec(12*PX, 17*PX),
	Vec(12*PX, 19*PX),
	Vec(4*PX, 19*PX),
};
Vec rata_poly_sensor_wall_l [] = {
	Vec(-4*PX, 19*PX),
	Vec(-12*PX, 19*PX),
	Vec(-12*PX, 17*PX),
	Vec(-4*PX, 17*PX),
};

FixProp rata_fixprop = {true, false, 1.0, 0, false};
FixProp rata_fixprop_feet = {false, true, 1.0, 0, false};
FixProp rata_fixprop_helmet = {true, true, 1.0, 0, false};
b2FixtureDef rata_fixes [] = {
	make_fixdef(make_poly(4, rata_poly_feet), cf::disabled, 0, 0, 1.0, &rata_fixprop_feet, false),
	make_fixdef(make_poly(7, rata_poly_27), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(7, rata_poly_25), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(7, rata_poly_21), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(4, rata_poly_h7), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(5, rata_poly_crawl_r), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(5, rata_poly_crawl_l), cf::disabled, 0, 0, 1.0, &rata_fixprop, false),
	make_fixdef(make_poly(4, rata_poly_sensor_21), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_floor_r), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_floor_l), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_block_r), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_block_l), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_wall_r), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_poly(4, rata_poly_sensor_wall_l), cf::sensor, 0, 0, 0.0, &rata_fixprop, true),
	make_fixdef(make_circle(7*PX, 0*PX, 23.5*PX), cf::disabled, 0, 0, 0.0, &rata_fixprop_helmet, false),
	make_fixdef(make_circle(7*PX, 0*PX, 16.5*PX), cf::disabled, 0, 0, 0.0, &rata_fixprop_helmet, false),
	make_fixdef(make_circle(7*PX, 4.5*PX, 11.5*PX), cf::disabled, 0, 0, 0.0, &rata_fixprop_helmet, false),
	make_fixdef(make_circle(7*PX, -4.5*PX, 11.5*PX), cf::disabled, 0, 0, 0.0, &rata_fixprop_helmet, false)
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
FixProp flyer_fixprop = {false, false, 1.0, 12, false};
b2FixtureDef flyer_fix = make_fixdef(make_circle(6*PX), cf::enemy, 0, 0.6, 6.0, &flyer_fixprop);
FixProp heart_fixprop = {false, false, 0.0, 0, false};
b2FixtureDef heart_fix = make_fixdef(make_rect(0.5, 0.5), cf::pickup, 0.8, 0, 0.1, &heart_fixprop);

const obj::Type obj::type [obj::n_types] = {

	{"Game", -1, NULL, 0, obj::ALLOC<Game>, -1},
	{"Rata", 18, rata_fixes, 10, obj::ALLOC<Rata>, -1},
	{"Room", 0, NULL, 0, obj::ALLOC<Room>, -1},
	{"Bullet", 1, &bullet_fix, -10, obj::ALLOC<Actor>, -1},
	{"Back Tiles", -1, NULL, 500, obj::ALLOC<TileLayer>, -1},
	{"Bullet Layer", -1, NULL, -200, obj::ALLOC<BulletLayer>, -1},
	{"Front Tiles", -1, NULL, -500, obj::ALLOC<TileLayer>, -1},
	{"Shade", -1, NULL, -3000, obj::ALLOC<Shade>, -1},
//	{"Door", -1, NULL, -100,  obj::ALLOC<Door>, -1},
//	{"Solid Object", -1, NULL, 0, obj::ALLOC<Solid>, -1},
//	{"Tilemap", 0, NULL, 0, obj::ALLOC<Tilemap>, -1},
//	{"Rat", 1, &rat_fix, 15, obj::ALLOC<Rat>, img::rat},
//	{"Crate", 1, &crate_fix, 0, obj::ALLOC<Crate>, img::crate},
//	{"Mousehole", 1, &mousehole_fix, 50, obj::ALLOC<Mousehole>, img::mousehole},
//	{"Hit Effect", -1, NULL, -90, obj::ALLOC<HitEffect>, -1},
//	{"Patroller", 1, patroller_fixes, 20, obj::ALLOC<Patroller>, img::patroller},
//	{"Flyer", 1, &flyer_fix, 20, obj::ALLOC<Flyer>, img::flyer},
//	{"Heart", 1, &heart_fix, -20, obj::ALLOC<Heart>, img::heart},
//	{"Item", -1, NULL, -5, obj::ALLOC<Item>, -1},
//	{"Lifebar", -1, NULL, -4000, obj::ALLOC<Lifebar>, -1},

};




#endif

