

struct Entrance : Object {
	void on_create () {
		if (room::entrance == (int)desc->data2) {
			if (rata->body) {
				rata->set_pos(desc->pos);
				if (desc->data) rata->state = (Rata::State)desc->data;
				if (desc->facing) rata->facing = desc->facing;
			}
			else {
				rata->desc->pos = desc->pos;
				if (desc->data) rata->desc->data = desc->data;
				if (desc->facing) rata->desc->facing = desc->facing;
			}
		}
		destroy();
	}
};

struct Exit : Object {
	void after_move () {
		if (rata->x() > desc->pos.x)
		if (rata->x() < desc->pos.x + desc->vel.x)
		if (rata->y() > desc->pos.y)
		if (rata->y() < desc->pos.y + desc->vel.y)
			enter_room(desc->data, desc->data2);
	}
};

struct Door : Object {
	void before_move () {
		if (rata->floor)
			rata->propose_action(Rata::action_enter, this, desc->pos, desc->vel.x);
	}
};


struct Tilemap : Object {
	void on_create () { }
};



struct Bullet : Object {
};



struct Crate : Object {
	virtual char* describe () { return "There's a wooden crate sitting here.\x80\nIt looks like it can be pushed around."; }
	virtual void on_create () { Object::on_create(); life = max_life = 144; }
	virtual void damage (int d) { Object::damage(d); snd::def[snd::woodhit].play(); }
};

struct Mousehole : Object {
	int timer;
	virtual char* describe () { return "A large metal pipe is coming out of the ground.\x80\nFor ventilation, perhaps?\x80\nIt's big enough for a rat to crawl through.\x80\nMaybe that's where they're coming from."; }
	virtual void before_move () {
		timer--;
		if (timer <= 0) {
			timer = 300 + rand() % 1200;
			if (rata && abs_f(rata->x() - desc->pos.x) < 3.0 && abs_f(rata->y() - desc->pos.y)) {
				return;
			}
			(new obj::Desc (-2, obj::rat, desc->pos))->manifest();
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
		timer = desc->vel.x;
		int16 image = img::hit_damagable;
		numsubs = img::def[image].numsubs();
		fpsub = (timer+numsubs-1) / numsubs;
	}
	virtual void draw () {
		int16 image = img::hit_damagable;
		uint sub = numsubs - timer / numsubs - 1;
		draw_image(image, desc->pos, sub);
		timer--;
		if (timer == 0) destroy();
	}
};

struct Heart : Object {
	virtual char* describe () { return "Just as rats live off the refuse of humans,\x80\nYou too can live off of the rats.\x80\nPick this up to restore one heart."; }
};

struct TileLayer : Object { };
struct BulletLayer : Object { };

struct Shade : Object {
	void draw () {
		draw_rect(
			desc->pos.x, desc->pos.y,
			desc->pos.x + desc->vel.x, desc->pos.y + desc->vel.y,
			desc->data, !desc->facing
		);
	}
};


struct Lifebar : Object {
	void draw () {
		if (rata) {
			int life = rata->life;
			for (int i = 0; i < (rata->max_life+47)/48; i++) {
				uint subimage;
				if (life >= i*48 + 48)
					subimage = 0;
				else if (life >= i*48 + 36)
					subimage = 1;
				else if (life >= i*48 + 24)
					subimage = 2;
				else if (life >= i*48 + 12)
					subimage = 3;
				else
					subimage = 4;
				draw_image(
					img::heart,
					Vec(19.5 - (i * 12*PX), 14.25),
					subimage,
					false, true
				);
			}
		}
	}
};















