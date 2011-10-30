


 // Base class for semi-intelligent enemies
struct AI : Walking {
	int decision_timer;
	virtual int update_start () { return 0; }
	virtual int update_interval () { return 30; }
	virtual Vec eye () { return Vec(x(), y()+1); }
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

	bool clear_to_point (Vec p) {
		return !look_line(eye(), p);
	}
	
	 // Sight and seeing
	bool see_rata_at (Vec p) {
		return !look_line(eye(), p, 2|32);
	}
	bool see_rata () {
		for (uint i=0; i < n_sight_points; i++)
		if (see_rata_at(rata->pos() + rata->sight_points[i]))
			return true;
		return false;
	}
	Vec see_rata_pos () {
		for (uint i=0; i < n_sight_points; i++)
		if (see_rata_at(rata->pos() + rata->sight_points[i]))
			return rata->pos() + rata->sight_points[i];
		return Vec::undef;
	}
	Vec predict_pos_from (Vec p, Object* threat = rata) {
		float lead = rand()*1.0/RAND_MAX;
		return Vec(p + lead * threat->vel() * decision_timer/FPS);
	}

	Vec predict_pos (Object* threat = rata) {
		float lead = rand()*1.0/RAND_MAX;
		return Vec(threat->pos() + lead * threat->vel() * decision_timer/FPS);
	}
};




struct Rat : AI {
	int anim_timer;
	char* describe () { return "It's a large gray rat that smells like trash.\x80\nDoesn't seem very timid for a rodent."; }
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
		Object::kill();
		snd::def[snd::squeak].play();
		if (rand() % 4 == 0)
			(new obj::Desc(-2, obj::heart, pos()))->manifest();
	}
};


struct Patroller : AI {
	virtual char* describe () { return "A small robot is patrolling the area.\x80\nIt has a gun attached.  Best be cautious."; }
	uint motion_frames;
	bool threat_detected;
	Vec threat_pos;
	Vec threat_vel;
	Vec prediction;
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
			if (clear_to_point(Vec(x()+0.7*facing, y()-0.3))
			 || !clear_to_point(Vec(x()+0.7*facing, y()+0.3))) {
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
			RBullet* b = fire_rbullet_to(
				this,
				Vec(x(), y()+0.5),
				Vec(prediction.x, prediction.y + 1),
				120, 48, 0.1
			);
			snd::def[snd::gunshot].play(1.0, 70);
			add_vel(Vec(-b->vel.x*FPS/60, 0));
		}
		Vec ratapos;
		threat_detected = (rata->x() - x())*facing > 0;
		if (threat_detected) {
			ratapos = see_rata_pos();
			if (defined(ratapos))
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
		snd::def[snd::hit].play(0.8);
	}
};

struct Flyer : AI {
	virtual char* describe () { return "An armed flying robot, small and deadly.\x80\nThat propellor lets it travel anywhere it wants to."; }
	uint motion_frames;
	Vec dest;
	Vec oldpos;
	Vec prediction;
	void on_create () {
		AI::on_create();
		life = max_life = 96;
		motion_frames = 0;
		body->SetGravityScale(0.0);
		//body->SetLinearDamping(0.2);
		dest = Vec(x(), y());
		prediction = Vec::undef;
		oldpos = prediction;
		decision_timer = rand()%40;
	}
	void before_move () {
		float accel = 8.0;
		float destdir = atan2(dest.y - y(), dest.x - x());
		Vec reldest = Vec(dest.x-x(), dest.y-y());
		float destdist = mag(reldest);
		float relv = dot(body->GetLinearVelocity(), norm(reldest));
		bool offtrack = (relv < mag(body->GetLinearVelocity()) - 1.0);
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
			body->ApplyForceToCenter(Vec(
				accel*cos(destdir),
				accel*sin(destdir)
			));
		}
		else {
			body->ApplyForceToCenter(Vec(
				-accel*cos(destdir),
				-accel*sin(destdir)
			));
		}
		AI::before_move();
	}

	Vec eye () { return Vec(x(), y()); }
	int update_interval () { return 40; }
	void decision () {
		Vec ratapos = see_rata_pos();
		 // Fire
		if (defined(prediction)) {
			RBullet* b = fire_rbullet_to(
				this,
				Vec(x(), y()),
				Vec(prediction.x, prediction.y),
				120, 48, 0.1
			);
			snd::def[snd::gunshot].play(1.0, 70);
			add_vel(-b->vel*FPS/60);
		}
		 // Predict
		if (defined(ratapos)) {
			//printf("Saw Rata at height %f\n", ratapos.y - rata->y());
			if (!defined(prediction))
				dest = Vec(x(), y());
			prediction = predict_pos_from(ratapos);
			oldpos = ratapos;
		}
		else {
			prediction = Vec::undef;
			 // Chase
			if (defined(oldpos)) {
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
		snd::def[snd::hit].play(0.8);
	}
};


