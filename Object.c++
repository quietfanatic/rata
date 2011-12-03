

#ifdef HEADER
struct Object : Spatial {
	Vec vel;
	b2Body* body;
	uint subimage;
	int life;
	int max_life;
	int facing;

	Object (int16 type, room::Def* loc, Vec pos, Vec vel = Vec(0, 0), int facing = 0);
	void activate ();
	void deactivate ();
	void before_move ();
	void after_move ();
	void draw ();

	virtual void damage (int d);
	virtual void heal (int d);
	virtual void kill ();
	virtual char* describe ();

	b2Body* make_body (b2BodyType btype = b2_staticBody, bool bullet=false);
	void impulse (Vec i);
	void mutual_impulse(Object* other, Vec i);
};


#else


Object::Object (int16 type, room::Def* loc, Vec pos, Vec vel, int facing) :
	Spatial(type, loc, pos),
	vel(vel),
	facing(facing),
	body(type::def[type].nfixes > 0 ? make_body(b2_dynamicBody, false) : NULL),
	life(0),
	max_life(0),
	subimage(0)
{
	for (int i = type::def[type].nfixes; i > 0; i--) {
		dbg_object("Fix %d: 0x%08x\n", i, body->CreateFixture(&(type::def[type].fixdef[i-1])));
	}
	dbg_object("Affixed 0x%08x with 0x%08x\n", this, body);
}

void Object::activate () {
	Spatial::activate();
	body->SetActive(true);
}
void Object::deactivate () {
	body->SetActive(false);
	Spatial::deactivate();
}

void Object::before_move () {
	if (pos != body->GetPosition())
		body->SetTransform(pos, 0);
	if (vel != body->GetLinearVelocity())
		body->SetLinearVelocity(vel);
}
void Object::after_move () {
	pos = body->GetPosition();
	vel = body->GetLinearVelocity();
	change_room();
}
void Object::draw () {
	img::Def* img = type::def[type].image;
	if (img)
	if (pos.x + img->w*PX >= camera.x - 10)
	if (pos.x - img->w*PX <= camera.x + 10)
	if (pos.y + img->h*PX >= camera.y - 7.5)
	if (pos.y - img->h*PX <= camera.y + 7.5)
		draw_image(type::def[type].image, pos, subimage, facing == 1);
}

 // New things specific to object.
void Object::damage (int d) { life -= d; if (life <= 0) kill(); }
void Object::heal (int h) { life += h; if (life > max_life) life = max_life; }
void Object::kill () { type = -1; deactivate(); }
char* Object::describe () { return "What a mysterious object."; }



 // Create a Box2d body for this object
b2Body* Object::make_body (b2BodyType btype, bool bullet) {
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

void Object::impulse (Vec i) {
	vel += i / body->GetMass();
}
void Object::mutual_impulse(Object* other, Vec i) {
	float m = body->GetMass();
	float om = other->body->GetMass();
	if (m == 0 || m == 1/0.0)
		other->impulse(i);
	if (om == 0 || om == 1/0.0)
		impulse(-i);
	else {
		other->impulse(i * m/(m+om));
		impulse(-i * om/(m+om));
	}
}


#endif

