


struct Rat : Walking {
	int decision_timer;
	int anim_timer;
	char* describe () { return "It's a large gray rat that smells like trash.\nDoesn't seem very timid for a rodent."; }
	bool is_damagable() { return true; }
	void on_create () {
		Object::on_create();
		Walking::on_create();
		if (!facing) facing = -1;
		decision_timer = 0;
		anim_timer = 0;
		life = max_life = 48;
		floor_friction = 10.0;
	}
	void before_move () {
		if (decision_timer == 0) {
			uint decision = rand();
			if (decision < RAND_MAX/3)
				ideal_xvel = 5.0;
			else if (decision < RAND_MAX/3*2)
				ideal_xvel = -5.0;
			else
				ideal_xvel = 0;
			decision_timer = (rand() % 10) * (rand() % 10);
		}
		else decision_timer--;
		Walking::before_move();
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
	virtual int touch_damage () { return 12; }
	virtual void kill () {
		destroy();
		if (rand() % 4 == 0)
			(new obj::Desc(
				obj::heart, NULL, x(), y(), 0, 0, 0, true
			))->manifest();
	}
};


struct Patroller : Walking {
	virtual char* describe () { return "A small robot is patrolling the area.\nIt has a gun attached.  Best be cautious."; }
	virtual int update_interval () { return 30; }
	virtual bool is_damagable () { return true; }
	uint lifetime;
	bool threat_detected;
	float threat_x;
	float threat_y;
	float threat_xvel;
	float threat_yvel;
	int motion_frames;
	virtual void on_create () {
		Object::on_create();
		Walking::on_create();
		life = max_life = 144;
		motion_frames = 0;
		lifetime = 0;
		threat_detected = false;
		if (!facing) facing = -1;
		floor_friction = 3.0;
		ideal_xvel = 3.0*facing;
	}
	float camera_x () { return x(); }
	float camera_y () { return y()+1; }
	virtual void before_move () {
		bool see_floor = check_line(camera_x(), camera_y(), x()+0.7*facing, y()-0.3);
		lifetime++;
		if (threat_detected) {
			ideal_xvel = 0.0;
		}
		else {
			if (!see_floor) {
				facing = -facing;
			}
			ideal_xvel = 3.0*facing;
		}
		if (lifetime % update_interval() == 0) {
			if (threat_detected) {
				float bullet_velocity = 120.0;
				float aim_direction = atan2(
					threat_y + threat_yvel*(update_interval()/FPS) - y(),
					threat_x + threat_xvel*(update_interval()/FPS) - x()
				);
				float bvx = bullet_velocity * cos(aim_direction);
				float bvy = bullet_velocity * sin(aim_direction);
				(new obj::Desc(obj::bullet, this,
					x(), y()+0.5, bvx, bvy, 0, true
				))->manifest();
				add_vel(-bvx/60, 0);
			}
			if ((threat_detected = detect_threat())) {
				threat_x = rata->x();
				threat_y = rata->y() + 8*PX;
				threat_xvel = rata->xvel() * (0.8 + rand()*0.2/RAND_MAX);
				threat_yvel = rata->yvel() * (0.8 + rand()*0.2/RAND_MAX);
			}
		}
		motion_frames += xvel()*facing;
		Walking::before_move();
	}
	virtual void draw () {
		motion_frames %= 60;
		subimage = (motion_frames > 30);
		Object::draw();
	}

	bool detect_threat () {
		if (rata && (rata->x() - x())*facing > 0) {
			return !check_line(camera_x(), camera_y(), rata->x(), rata->y() + 8*PX, 2|32);
		}
		return false;
	}
};

