


struct Rat : Damagable {
	int decision_timer;
	int anim_timer;
	int facing;
	char* describe () { return "It's a large gray rat that smells like trash.\nDoesn't seem very timid for a rodent."; }
	void on_create () {
		Damagable::on_create();
		decision_timer = 0;
		anim_timer = 0;
		facing = -1;
		life = max_life();
	}
	void before_move () {
		if (decision_timer == 0) {
			uint decision = rand();
			if (decision < RAND_MAX/3) {
				body->SetLinearVelocity(b2Vec2(5.0, yvel()));
				facing = 1;
			}
			else if (decision < RAND_MAX/3*2) {
				body->SetLinearVelocity(b2Vec2(-5.0, yvel()));
				facing = -1;
			}
			else {
				body->SetLinearVelocity(b2Vec2(0.0, yvel()));
			}
			decision_timer = (rand() % 10) * (rand() % 10);
		}
		else decision_timer--;
	}
	void draw () {
		if (anim_timer < 6) {
			draw_image_sub(img::rat, x(), y(), 0, 0, 16, 8, facing == 1);
		}
		else {
			draw_image_sub(img::rat, x(), y(), 0, 8, 16, 16, facing == 1);
		}
		if (abs_f(xvel()) > 0.1) {
			anim_timer++;
			anim_timer %= 12;
		}
	}
	virtual int touch_damage () { return 12; }
	virtual void on_destroy () {
		if (rand() % 4 == 0)
			(new obj::Desc(
				obj::heart, NULL, x(), y()+4*PX, 0, 0, 0, true
			))->manifest();
	}
};


struct Patroller : Damagable {
	virtual char* describe () { return "A small robot is patrolling the area.\nIt has a gun attached. Best be cautious."; }
	virtual int update_interval () { return 30; }
	virtual int max_life () { return 144; }
	uint lifetime;
	bool threat_detected;
	float threat_x;
	float threat_y;
	float threat_xvel;
	float threat_yvel;
	int motion_frames;
	virtual void on_create () {
		Damagable::on_create();
		motion_frames = 0;
		lifetime = 0;
		threat_detected = false;
		if (!facing) facing = -1;
		set_vel(3.0*facing, 0.0);
	}
	virtual void before_move () {
		lifetime++;
		if (threat_detected) {
			set_vel(0.0, yvel());
		}
		else {
			Object* floor = check_area(
			 	x() + 6*PX*facing, y() - 14*PX,
				x() + 14*PX*facing, y() - 6*PX
			);
			if (xvel()*facing < 0.1 || !floor) {
				facing = -facing;
				set_vel(3.0*facing, 0.0);
			}
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
					x(), y(), bvx, bvy, 0, true
				))->manifest();
				add_vel(-bvx/60, 0);
			}
			if ((threat_detected = detect_threat())) {
				threat_x = rata->x();
				threat_y = rata->y();
				threat_xvel = rata->xvel() * (0.8 + rand()*0.2/RAND_MAX);
				threat_yvel = rata->yvel() * (0.8 + rand()*0.2/RAND_MAX);
			}
		}
		motion_frames += xvel()*facing;
	}
	virtual void draw () {
		motion_frames %= 60;
		if (motion_frames > 30)
			draw_image_sub(
				img::patroller, x(), y(),
				0, 0, 16, 16, (facing == 1)
			);
		else
			draw_image_sub(
				img::patroller, x(), y(),
				0, 16, 16, 32, (facing == 1)
			);
	}

	bool detect_threat () {
		if (rata && (rata->x() - x())*facing > 0) {
			return !check_line(x(), y(), rata->x(), rata->y(), 2|32);
		}
		return false;
	}
};

