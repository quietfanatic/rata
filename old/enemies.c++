
 // Base class for semi-intelligent enemies
struct AI : Walking {
	int decision_timer;

	virtual int update_start () { return rand()%update_interval(); }
	virtual int update_interval () { return 30; }
	virtual Vec eye () { return pos + Vec(0, 1); }
	virtual void decision () { }
	
	AI (int16 type, room::Def* loc, Vec pos, Vec vel = Vec(0, 0)) :
		Walking(type, loc, pos, vel),
		decision_timer(update_start())
	{ }
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
		if (see_rata_at(rata->pos + rata->sight_points[i]))
			return true;
		return false;
	}
	Vec see_rata_pos () {
		for (uint i=0; i < n_sight_points; i++)
		if (see_rata_at(rata->pos + rata->sight_points[i]))
			return rata->pos + rata->sight_points[i];
		return Vec::undef;
	}
	Vec predict_pos_from (Vec p, Object* threat = rata) {
		float lead = rand()*1.0/RAND_MAX;
		return Vec(p + lead * threat->vel * decision_timer/FPS);
	}

	Vec predict_pos (Object* threat = rata) {
		float lead = rand()*1.0/RAND_MAX;
		return Vec(threat->pos + lead * threat->vel * decision_timer/FPS);
	}
};




struct Rat : AI {
	int anim_timer;
	char* describe () { return "It's a large gray rat that smells like trash.\x80\nDoesn't seem very timid for a rodent."; }
	Rat (room::Def* loc, Vec pos, Vec vel = Vec(0, 0)) :
		AI(type::rat, loc, pos, vel),
		anim_timer(0)
	{
		if (!facing) facing = -1;
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
		if (vel.x*facing < -0.01) facing = -facing;
		subimage = (anim_timer > 5);
		if (abs_f(vel.x) > 0.1) {
			anim_timer++;
			anim_timer %= 12;
		}
		Object::draw();
	}
	void kill () {
		Object::kill();
		snd::def[snd::squeak].play();
		if (rand() % 4 == 0)
			; //Drop heart
	}
};


struct Patroller : AI {
	char* describe () { return "A small robot is patrolling the area.\x80\nIt has a gun attached.  Best be cautious."; }
	uint motion_frames;
	bool threat_detected;
	Vec threat_pos;
	Vec threat_vel;
	Vec prediction;

	Patroller (room::Def* loc, Vec pos, Vec vel = Vec(0, 0)) :
		AI(type::patroller, loc, pos, vel),
		motion_frames(0),
		threat_detected(false)
	{
		life = max_life = 144;
		if (!facing) facing = -1;
		floor_friction = 3.0;
		ideal_xvel = 3.0*facing;
	}
	void before_move () {
		if (threat_detected) {
			ideal_xvel = 0.0;
		}
		else if (floor) {
			if (clear_to_point(pos + Vec(0.7*facing, -0.3))
			 || !clear_to_point(pos + Vec(0.7*facing, 0.3))) {
				facing = -facing;
			}
			ideal_xvel = 3.0*facing;
		}
		motion_frames += vel.x*facing;
		AI::before_move();
	}
	void decision () {
		if (threat_detected) {
			RBullet* b = fire_rbullet_to(
				this,
				pos + Vec(0, 0.5),
				prediction + Vec(0, 1),
				120, 48, 0.1
			);
			snd::def[snd::gunshot].play(1.0, 70);
			vel.x -= b->vel.x*FPS/60;
		}
		Vec ratapos;
		threat_detected = (rata->pos.x - pos.x)*facing > 0;
		if (threat_detected) {
			ratapos = see_rata_pos();
			if (defined(ratapos))
				prediction = predict_pos();
			else threat_detected = false;
		}
	}
	void after_move () {
		AI::after_move();
		if (threat_detected) {
			propose_attention(Attention(
				2000 - mag2(eye() - rata->cursor_pos()),
				Rect(pos - Vec(9.5, 6.5), pos + Vec(9.5, 7.5))
			));
		}
	}
	void draw () {
		motion_frames %= 60;
		subimage = (motion_frames < 30);
		Object::draw();
	}

	void damage (int d) {
		Object::damage(d);
		snd::def[snd::hit].play(0.8);
	}
};

struct Flyer : AI {
	virtual char* describe () { return "An armed flying robot, small and deadly.\x80\nThat propellor lets it travel anywhere it wants to."; }
	Vec dest;
	Vec oldpos;
	Vec prediction;
	uint motion_frames;
	int angle_frame;

	Flyer (room::Def* loc, Vec pos, Vec vel = Vec(0, 0)) :
		AI(type::flyer, loc, pos, vel),
		dest(pos),
		oldpos(Vec::undef),
		prediction(Vec::undef),
		motion_frames(0),
		angle_frame(2)
	{
		life = max_life = 96;
		body->SetGravityScale(0.0);
	}
	void before_move () {
		float accel = 8.0;
		Vec reldest = dest - pos;
		float destdir = ang(dest - pos);
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
			destdir = ang(-vel);
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

	Vec eye () { return pos; }
	int update_interval () { return 40; }
	void decision () {
		Vec ratapos = see_rata_pos();
		 // Fire
		if (defined(prediction)) {
			RBullet* b = fire_rbullet_to(
				this,
				pos,
				prediction,
				120, 48, 0.1
			);
			snd::def[snd::gunshot].play(1.0, 70);
			vel -= b->vel*FPS/60;
		}
		 // Predict
		if (defined(ratapos)) {
			//printf("Saw Rata at height %f\n", ratapos.y - rata->y());
			if (!defined(prediction))
				dest = pos;
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
	void after_move () {
		AI::after_move();
		if (defined(prediction) || mag2(dest - pos) > 1) {
			propose_attention(Attention(
				1000 - mag2(eye() - rata->cursor_pos()),
				Rect(pos - Vec(9.5, 7.0), pos + Vec(9.5, 7.0))
			));
		}
	}
	void draw () {
		motion_frames++;
		motion_frames %= 4;
		if (defined(prediction)) {
			angle_frame = get_angle_frame(ang(prediction - pos));
			if (prediction.x < pos.x)
				facing = -1;
			else facing = 1;
		}
		subimage = angle_frame + 9 * (motion_frames < 2);
		Object::draw();
	}
	void damage (int d) {
		Object::damage(d);
		snd::def[snd::hit].play(0.8);
	}
};

