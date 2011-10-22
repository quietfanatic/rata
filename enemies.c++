


 // Base class for semi-intelligent enemies
struct AI : Walking {
	int decision_timer;
	virtual int update_start () { return 0; }
	virtual int update_interval () { return 30; }
	virtual float eyex () { return x(); }
	virtual float eyey () { return y() + 1; }
	virtual void decision () { }

	void on_create () {
		Object::on_create();
		Walking::on_create();
		decision_timer = update_start();
	}
	void before_move () {
		if (decision_timer <= 0) {
			decision_timer = update_interval();
			decision();
		}
		decision_timer--;
		Walking::before_move();
	}

	bool clear_to_point (float x, float y) {
		return !check_line(eyex(), eyey(), x, y);
	}
	
	 // Sight and seeing
	bool see_rata_at (float x, float y) {
		return check_line(eyex(), eyey(), x, y, 1|2|32) == rata;
	}
	bool see_rata () {
		return see_rata_at(rata->x(), rata->y()+0.8)
		    || see_rata_at(rata->x(), rata->y()+1.6)
		    || see_rata_at(rata->x(), rata->y()+0.2);
	}
	b2Vec2 see_rata_pos () {
		return see_rata_at(rata->x(), rata->y()+0.8) ? b2Vec2(rata->x(), rata->y()+0.8)
		     : see_rata_at(rata->x(), rata->y()+1.6) ? b2Vec2(rata->x(), rata->y()+1.6)
		     : see_rata_at(rata->x(), rata->y()+0.2) ? b2Vec2(rata->x(), rata->y()+0.2)
		     : b2Vec2(0/0.0, 0/0.0);
	}
	b2Vec2 predict_pos_from (float x, float y, Object* threat = rata) {
		return b2Vec2(x + threat->xvel() * decision_timer/FPS,
		              y + threat->yvel() * decision_timer/FPS);
	}

	b2Vec2 predict_pos (Object* threat = rata) {
		return b2Vec2(threat->x() + threat->xvel() * decision_timer/FPS,
		              threat->y() + threat->yvel() * decision_timer/FPS);
	}
};




struct Rat : AI {
	int anim_timer;
	char* describe () { return "It's a large gray rat that smells like trash.\nDoesn't seem very timid for a rodent."; }
	void on_create () {
		AI::on_create();
		if (!facing) facing = -1;
		anim_timer = 0;
		life = max_life = 48;
		floor_friction = 10.0;
	}
	int update_interval () { return (rand() % 10) * (rand() % 10); }
	void decision () {
		uint decision = rand();
		if (decision < RAND_MAX/3)
			ideal_xvel = 5.0;
		else if (decision < RAND_MAX/3*2)
			ideal_xvel = -5.0;
		else
			ideal_xvel = 0;
	}
	void draw () {
		if (xvel()*facing < -0.01) facing = -facing;
		subimage = (anim_timer > 5);
		if (abs_f(xvel()) > 0.1) {
			anim_timer++;
			anim_timer %= 12;
		}
		Object::draw();
	}
	virtual void kill () {
		destroy();
		snd::squeak.play();
		if (rand() % 4 == 0)
			(new obj::Desc(
				obj::heart, NULL, x(), y(), 0, 0, 0, true
			))->manifest();
	}
};


struct Patroller : AI {
	virtual char* describe () { return "A small robot is patrolling the area.\nIt has a gun attached.  Best be cautious."; }
	uint motion_frames;
	bool threat_detected;
	float threat_x;
	float threat_y;
	float threat_xvel;
	float threat_yvel;
	b2Vec2 prediction;
	virtual void on_create () {
		AI::on_create();
		life = max_life = 144;
		motion_frames = 0;
		threat_detected = false;
		if (!facing) facing = -1;
		floor_friction = 3.0;
		ideal_xvel = 3.0*facing;
		decision_timer = rand()%30;
	}
	virtual void before_move () {
		if (threat_detected) {
			ideal_xvel = 0.0;
		}
		else if (floor) {
			if (clear_to_point(x()+0.7*facing, y()-0.3)
			 || !clear_to_point(x()+0.7*facing, y()+0.3)) {
				facing = -facing;
			}
			ideal_xvel = 3.0*facing;
		}
		AI::before_move();
		motion_frames += xvel()*facing;
		Walking::before_move();
	}
	void decision () {
		if (threat_detected) {
			Bullet* b = fire_bullet_to(
				x(), y()+0.5,
				prediction.x,
				prediction.y + 1,
				120, 48, 0.1
			);
			snd::gunshot.play(1.0, 70);
			add_vel(-b->desc->xvel/60, 0);
		}
		b2Vec2 ratapos;
		threat_detected = (rata->x() - x())*facing > 0;
		if (threat_detected) {
			ratapos = see_rata_pos();
			if (ratapos.x == ratapos.x)
				prediction = predict_pos();
			else threat_detected = false;
		}
	}
	virtual void draw () {
		motion_frames %= 60;
		subimage = (motion_frames > 30);
		Object::draw();
	}

	void damage (int d) {
		Object::damage(d);
		snd::hit.play(0.8);
	}
};

struct Flyer : AI {
	uint motion_frames;
	b2Vec2 dest;
	b2Vec2 oldpos;
	b2Vec2 prediction;
	void on_create () {
		AI::on_create();
		life = max_life = 96;
		motion_frames = 0;
		body->SetGravityScale(0.0);
		//body->SetLinearDamping(0.2);
		dest = b2Vec2(x(), y());
		prediction = b2Vec2(0/0.0, 0/0.0);
		oldpos = prediction;
		decision_timer = rand()%40;
	}
	void before_move () {
		float accel = 8.0;
		float destdir = atan2(dest.y - y(), dest.x - x());
		b2Vec2 reldest = b2Vec2(dest.x-x(), dest.y-y());
		float destdist = reldest.Length();
		reldest.Normalize();
		float relv = dot(body->GetLinearVelocity(), reldest);
		bool offtrack = (relv < body->GetLinearVelocity().Length() - 1.0);
		// We have: p, v, a.  We need d.
		// d = p + vt + att/2; since v = at, t = v/a
		// d = p + vv/a + vv/a/2;
		// d = p + 3vv/2a
		// Weird, but that's what the math says.
		float stopdist = 1.5 * relv*relv/accel;
		//printf("%f <= %f == %d\n", stopdist, destdist, stopdist<=destdist);
		if (offtrack) {
			destdir = atan2(-yvel(), -xvel());
		}
		//destdir = dither(destdir, 0.01);
		if (offtrack || stopdist <= destdist) {
			body->ApplyForceToCenter(b2Vec2(
				accel*cos(destdir),
				accel*sin(destdir)
			));
		}
		else {
			body->ApplyForceToCenter(b2Vec2(
				-accel*cos(destdir),
				-accel*sin(destdir)
			));
		}
		AI::before_move();
	}

	float eyex () { return x(); }
	float eyey () { return y(); }
	int update_interval () { return 40; }
	void decision () {
		b2Vec2 ratapos = see_rata_pos();
		 // Fire
		if (prediction.IsValid()) {
			Bullet* b = fire_bullet_to(
				x(), y(),
				prediction.x,
				prediction.y,
				120, 48, 0.1
			);
			snd::gunshot.play(1.0, 70);
			add_vel(-b->desc->xvel/120, -b->desc->yvel/120);
		}
		 // Predict
		if (ratapos.IsValid()) {
			//printf("Saw Rata at height %f\n", ratapos.y - rata->y());
			dest = b2Vec2(x(), y());
			prediction = predict_pos_from(ratapos.x, ratapos.y);
			oldpos = ratapos;
		}
		else {
			prediction = b2Vec2(0/0.0, 0/0.0);
			 // Chase
			if (oldpos.IsValid()) {
				dest = oldpos;
			}
		}
	}
	void draw () {
		motion_frames++;
		motion_frames %= 4;
		subimage = (motion_frames < 2);
		Object::draw();
	}
	void damage (int d) {
		Object::damage(d);
		snd::hit.play(0.8);
	}
};


