#define RATA_STEP 1.0
#define MAX_EQUIPS 10


struct Rata : Walking {
	 // Character state
	//Object* floor;
	int float_frames;
	bool aiming;
	bool can_see;
	Object* pointed_object;
	bool take_damage;
	int hurt_frames;
	int inv_frames;
	int adrenaline;
	bool sitting;
	bool dead;
	bool kneeling;
	bool crawling;
	float aim_distance;
	float aim_direction;
	float distance_walked;  // For drawing
	float oldxrel;
	int recoil_frames;
	float handx;
	float handy;
	uint angle_frame;  // 0=down, 8=up

	item::Equip* equipment [MAX_EQUIPS];

	 // Easy access to bits
	float aim_center_x () { return x() + 2*PX*facing; }
	float aim_center_y () { return y() + 13*PX; }
	b2Fixture* fix_27 () { return body->GetFixtureList(); }
	b2Fixture* fix_25 () { return body->GetFixtureList()->GetNext(); }
	b2Fixture* fix_21 () { return body->GetFixtureList()->GetNext()->GetNext(); }
	b2Fixture* fix_h7 () { return body->GetFixtureList()->GetNext()->GetNext()->GetNext(); }

//	b2Fixture* fix_current () {
//		if (hurt_frames) return fix_27();
//		else if (kneeling) return fix_21();
//		else return fix_25();
//	}

	void set_fix_27 () {
		fix_27()->SetFilterData(inv_frames ? cf::rata_invincible : cf::rata);
		fix_25()->SetFilterData(cf::disabled);
		fix_21()->SetFilterData(cf::disabled);
		fix_h7()->SetFilterData(cf::disabled);
	}
	void set_fix_25 () {
		fix_27()->SetFilterData(cf::disabled);
		fix_25()->SetFilterData(inv_frames ? cf::rata_invincible : cf::rata);
		fix_21()->SetFilterData(cf::disabled);
		fix_h7()->SetFilterData(cf::disabled);
	}
	void set_fix_21 () {
		fix_27()->SetFilterData(cf::disabled);
		fix_25()->SetFilterData(cf::disabled);
		fix_21()->SetFilterData(inv_frames ? cf::rata_invincible : cf::rata);
		fix_h7()->SetFilterData(cf::disabled);
	}
	void set_fix_h7 () {
		fix_27()->SetFilterData(cf::disabled);
		fix_25()->SetFilterData(cf::disabled);
		fix_21()->SetFilterData(cf::disabled);
		fix_h7()->SetFilterData(inv_frames ? cf::rata_invincible : cf::rata);
	}

	 // Character stats (affected by items and such)
	float ground_accel () {
		return 0.5;
	}
	float ground_decel () {
		return 1.0;
	}
	float max_forward_speed () {
		return 6.0;
	}
	float max_backward_speed () {
		return 4.0;
	}
	float max_crawl_speed () {
		return 2.0;
	}
	float air_accel () {
		return 0.2;
	}
	float max_air_speed () {
		return 4.0;
	}
	float jump_velocity () {
		return 10.0;
	}
	float jump_float_time () {
		return 60;
	}
	float max_fall_speed () {
		return 8.0;
	}
	virtual bool is_damagable () { return true; }
	
	void get_aim () {
		aiming = (button[sf::Mouse::Right] > 0 || key[sf::Key::LShift] > 0);
		if (cursor.x > 0) facing = 1;
		else if (cursor.x < 0) facing = -1;
		aim_distance = sqrt(cursor.x*cursor.x + cursor.y*cursor.y);
		aim_direction = atan2(cursor.y, cursor.x);
		pointed_object = check_area(
			aim_center_x() + cursor.x + 1*PX, aim_center_y() + cursor.y + 1*PX,
			aim_center_x() + cursor.x - 1*PX, aim_center_y() + cursor.y - 1*PX,
			1|2|4|8|16|32|64
		);
		 // Check if we can see where we're pointing
		if (!aiming) {
			if (abs_f(cursor.x) < 0.1 && abs_f(cursor.y) < 0.1) {
				can_see = true;
			}
			else {
				Object* seeing = check_line(
					aim_center_x(), aim_center_y(), aim_center_x()+cursor.x, aim_center_y()+cursor.y
				);
				can_see = (seeing == NULL);
			}
		}
	}

	void before_move () {
		//floor = get_floor(fix_feet_current());
		 // Aiming
		
		if (!dead) get_aim();

		if (take_damage) {
			if (life <= 0) add_vel(0, 5.0);
			else if (floor) add_vel(0, 3.0);
			floor = NULL;
			printf("Ouch!\n");
			set_fix_25();
		}
		 // Movement
		if (floor && !take_damage) {
			if (hurt_frames == 0) sitting = false;
			if (dead) {
				floor_friction = ground_decel();
				ideal_xvel = 0;
				set_fix_h7();
			}
			else if (sitting) {
				floor_friction = ground_decel();
				ideal_xvel = 0;
				set_fix_21();
			}
			else if (hurt_frames < 20) {
				if (hurt_frames > 0) {
					hurt_frames = 0;
				}
				oldxrel = x() - floor->x();
				 // Kneel
				kneeling = false;
				if (key[sf::Key::S] && floor_normal.y > 0.9) {
					ideal_xvel = 0;
					floor_friction = ground_decel()*ground_decel();
					kneeling = true;
					if (key[sf::Key::A] && !key[sf::Key::D]) {
						if (!crawling) facing = -1;
						crawling = true;
						ideal_xvel = -max_crawl_speed();
					}
					else if (key[sf::Key::D] && !key[sf::Key::A]) {
						if (!crawling) facing = 1;
						crawling = true;
						ideal_xvel = max_crawl_speed();
					}
					if (crawling) set_fix_h7();
					else set_fix_21();
				}
				 // Left
				else if (key[sf::Key::A] && !key[sf::Key::D]) {
					if (xvel() < 0)
						floor_friction = ground_accel();
					else floor_friction = ground_decel();
					if (facing < 0)
						ideal_xvel = -max_forward_speed();
					else
						ideal_xvel = -max_backward_speed();
					set_fix_27();
				}
				 // Right
				else if (key[sf::Key::D] && !key[sf::Key::A]) {
					if (xvel() > 0)
						floor_friction = ground_accel();
					else floor_friction = ground_decel();
					if (facing > 0)
						ideal_xvel = max_forward_speed();
					else
						ideal_xvel = max_backward_speed();
					set_fix_27();
				}
				 // Stop
				else {
					floor_friction = ground_decel();
					ideal_xvel = 0;
					set_fix_27();
				}
				 // Jump
				if (key[sf::Key::W] && key[sf::Key::W] == 1) {  // Jump
					//mutual_impulse(floor, 0, -jump_velocity()*mass());
					set_vel(xvel(), jump_velocity());
					float_frames = jump_float_time();
					set_fix_27();
				}
				if (!kneeling) crawling = false;
			}
			else {
				set_fix_21();
				sitting = true;
			}
		}
		else if (!dead && !hurt_frames) {  // Midair
			sitting = false;
			 // Left
			if (key[sf::Key::A] && !key[sf::Key::B]) {
				float max = -max_air_speed();
				if (xvel() > max) {
					add_vel(-air_accel(), 0);
					if (xvel() < max) {
						add_vel(-xvel() + max, 0);
					}
				}
				set_fix_27();
			}
			 // Right
			else if (key[sf::Key::D]) {
				float max = max_air_speed();
				if (xvel() < max) {
					add_vel(air_accel(), 0);
					if (xvel() > max) {
						add_vel(-xvel() + max, 0);
					}
				}
				set_fix_27();
			}
			 // Adjust falling speed
			if (key[sf::Key::W]
				&& float_frames
			) {
					body->SetGravityScale((jump_float_time()-float_frames)/jump_float_time());
					float_frames--;
			}
			else {
				body->SetGravityScale(1.0);
				float_frames = 0;
			}
			if (yvel() < 0.12 && yvel() > -0.12)
				dbg(2, "%f\n", body->GetPosition().y);
		}
		if (yvel() < -24) hurt_frames += 2;
		 // Use weapon
		if (aiming
		 && recoil_frames == 0
		 && button[sf::Mouse::Left]
		 && !hurt_frames) {
			for (uint i=0; i<MAX_EQUIPS; i++)
			if (equipment[i])
			if (equipment[i]->use)
				(*equipment[i]->use)();
		}
		else if (recoil_frames) recoil_frames--;
		 // Describe object or advance message
		if (!aiming && !dead) {
			if (button[sf::Mouse::Left] == 1) {
				if (message) {
					if (message_pos_next) {
						message_pos = message_pos_next;
						message_pos_next = NULL;
						for (char* p=message_pos; *p; p++) {  // Find next pos
							if (*p == '\n') {
								message_pos_next = p+1;
								break;
							}
						}
					}
					else {
						message = message_pos = NULL;
					}
				}
				else if (pointed_object && pointed_object->desc->id != obj::tilemap) {
					message = pointed_object->describe();
					message_pos = message;
					message_pos_next = NULL;
					for (char* p=message_pos; *p; p++) {  // Find next pos
						if (*p == '\n') {
							message_pos_next = p+1;
							break;
						}
					}
				}
			}
		}
		else if (message) { message = message_pos = message_pos_next = NULL; }
		 // Select cursor image
		if (aiming) cursor.img = &img::target;
		else if (message) cursor.img = &img::readmore;
		else if (can_see) {
			if (pointed_object && pointed_object->desc->id != obj::tilemap)
				cursor.img = &img::see;
			else cursor.img = &img::look;
		}
		else cursor.img = &img::nolook;
		 // Request debug information
//		if (key[sf::Key::BackSlash] == 1)
//			print_debug_all();
		if (hurt_frames && !dead) hurt_frames--;
		if (inv_frames && !dead) inv_frames--;
		if (adrenaline) adrenaline--;
		take_damage = false;
		Walking::before_move();
	}

	virtual void damage (int d) {
		if (!hurt_frames && !inv_frames) {
			Object::damage(d);
			take_damage = true;
			hurt_frames = 6 + d / (2 + adrenaline/60.0);
			inv_frames = hurt_frames + 20;
			adrenaline += 5*d;
		}
	}

	virtual void kill () {
		dead = true;
		message = message_pos = message_pos_next = NULL;
	}




	void after_move () {
//		printf("%08x's floor is: %08x\n", this, floor);
//		floor = get_floor(fix_feet_current());
		if (floor) {
			if (abs_f(xvelrel(floor)) < 0.1)
				distance_walked = 0;
			else
				distance_walked += abs_f((x() - floor->x()) - oldxrel);
		}
		else distance_walked = 0;
		 // Constrain cursor to room
		if (cursor.x + aim_center_x() > room::current->width) {
			cursor.y *= (room::current->width - aim_center_x()) / cursor.x;
			cursor.x = room::current->width - aim_center_x();
		}
		if (cursor.x + aim_center_x() < 0) {
			cursor.y *= (-aim_center_x()) / cursor.x;
			cursor.x = -aim_center_x();
		}
		if (cursor.y + aim_center_y() > room::current->height) {
			cursor.x *= (room::current->height - aim_center_y()) / cursor.y;
			cursor.y = room::current->height - aim_center_y();
		}
		if (cursor.y + aim_center_y() < 0) {
			cursor.x *= (-aim_center_y()) / cursor.y;
			cursor.y = -aim_center_y();
		}
	};

	virtual void on_create () {
		make_body(desc, true, true);
			for (uint i = obj::def[desc->id].nfixes; i > 0; i--) {
				dbg(4, "Fix %d: 0x%08x\n", i, body->CreateFixture(&(obj::def[desc->id].fixdef[i-1])));
			}
		set_fix_27();
		dbg(3, "Affixed 0x%08x with 0x%08x\n", this, body);
		floor = NULL;
		Walking::on_create();
		float_frames = 0;
		recoil_frames = 0;
		take_damage = false;
		hurt_frames = 0;
		inv_frames = 0;
		adrenaline = 0;
		sitting = kneeling = dead = false;
		life = max_life = 144;
		for (uint i=0; i<MAX_EQUIPS; i++) equipment[i] = NULL;
		equipment[0] = &item::white_dress;
		equipment[1] = &item::handgun;
		camera.x = x();
		camera.y = y();
		if (camera.x < 10) camera.x = 10;
		if (camera.x > room::current->width - 10) camera.x = room::current->width - 10;
		if (camera.y < 7.5) camera.y = 7.5;
		if (camera.y > room::current->height - 7.5) camera.y = room::current->height - 7.5;
		cursor.y = 0;
		facing = desc->facing || 1;
		printf("FACING = %d\n", facing);
		cursor.x = 2.0 * facing;
		cursor.img = &img::look;
		rata = this;
	}
	virtual void on_destroy () {
		rata = NULL;
	}

	void draw () {

		int walk_frame;
		float aim_angle;
		uint headpose;  // Pose index
		uint bodypose;
		int armpose;
		int forearmpose;
		int handpose;
		bool flip = facing<0;
		bool armflip = flip;
		bool forearmflip = flip;
		bool deadframe = dead && floor && !take_damage;

		//if (inv_frames % 3 == 2) return;

		 // Select aim angle frame
		if (facing > 0)
			aim_angle = aim_direction;
		else if (aim_direction > 0)
			aim_angle = M_PI - aim_direction;
		else
			aim_angle = -M_PI - aim_direction;

		angle_frame =
		  aim_angle > M_PI* 7.0/16.0 ? 8
		: aim_angle > M_PI* 5.0/16.0 ? 7
		: aim_angle > M_PI* 3.0/16.0 ? 6
		: aim_angle > M_PI* 1.0/16.0 ? 5
		: aim_angle > M_PI*-1.0/16.0 ? 4
		: aim_angle > M_PI*-3.0/16.0 ? 3
		: aim_angle > M_PI*-5.0/16.0
		  || kneeling                ? 2
		: aim_angle > M_PI*-7.0/16.0 ? 1
		:                              0;

		 // Select walking frame
		if (floor) {
			float step_d = MOD(distance_walked, RATA_STEP * 2);
			walk_frame =
			  distance_walked < 0.01    ? 0
			: step_d < RATA_STEP*5/9.0  ? 1
			: step_d < RATA_STEP        ? 2
			: step_d < RATA_STEP*14/9.0 ? 3
			:                             2;
		}
		else walk_frame = 3;

		 // Select body pose
		{ using namespace pose::body;
			bodypose =
			  deadframe        ? laybk
			: sitting          ? sit
			: hurt_frames      ? hurtbk
			: crawling ? (walk_frame % 2) ? crawl2
			           :                    crawl
			: kneeling         ? kneel
			: (walk_frame % 2) ? walk
			:                    stand;
		}

		 // Select head pose
		{ using namespace pose::head;
			headpose =
			  deadframe      ? laybk
			: sitting        ? stand_90
			: hurt_frames    ? hurtbk
			: crawling       ? crawl
			: walk_frame % 2
			   && floor      ? angle_walk[angle_frame]
			: yvel() < -1.0  ? angle_walk[angle_frame]
			:                  angle_stand[angle_frame];
		}

		 // Select arm pose
		{ using namespace pose::arm;
			armpose =
			  deadframe       ? a90
			: sitting         ? -a23
			: hurt_frames     ? a23
			: crawling ? walk_frame == 1 ? -a45
			           : walk_frame == 2 ? a0
			           : walk_frame == 3 ? a45
			           :                   a0
			: aiming ? recoil_frames > 20 ? angle_frame - 3
			         : aim_distance > 10  ? angle_frame
			         : aim_distance > 4   ? angle_frame == 0 ? 0
			                              :                    angle_frame - 2
			         :                      angle_frame == 0 ? 0
			                              : angle_frame == 1 ? -1
			                              :                    angle_frame - 4
			: kneeling        ? a45
			: walk_frame == 1 ? -a23
			: walk_frame == 2 ? a0
			: walk_frame == 3 ? a23
			:                   a0;
			if (armpose < a0) armpose = -armpose, armflip = !armflip;
			if (armpose > a180) armpose = (2*a180)-armpose, armflip = !armflip;
		}

		 // Select forearm pose
		{ using namespace pose::forearm;
			forearmpose =
			  deadframe       ? a90
			: sitting         ? -a23
			: hurt_frames     ? a68
			: crawling        ? a90
			: aiming ? recoil_frames > 20 ? angle_frame + 1
			         :                      angle_frame
			: kneeling        ? a68
			: walk_frame == 1 ? -a23
			: walk_frame == 2 ? a0
			: walk_frame == 3 ? a23
			:                   a0;
			if (forearmpose < a0) forearmpose = -forearmpose, forearmflip = !forearmflip;
			if (forearmpose > a180) forearmpose = (2*a180)-forearmpose, forearmflip = !forearmflip;
		}

		 // Select hand pose
		{ using namespace pose::hand;
			handpose =
			  deadframe       ? inside
			: sitting         ? front
			: hurt_frames     ? a68
			: crawling        ? inside
			: aiming ? recoil_frames > 20 ? angle_frame + 1
			         :                      angle_frame
			: kneeling        ? a45
			: walk_frame == 1 ? a338
			: walk_frame == 2 ? a0
			: walk_frame == 3 ? a23
			:                   a0;
		}
		
		 // Get position of various parts.
		
		float headx = pose::body::headx[bodypose]*facing;
		float heady = pose::body::heady[bodypose];
		float armx = pose::body::armx[bodypose]*facing;
		float army = pose::body::army[bodypose];
		float forearmx = armx + pose::arm::forearmx[armpose]*(armflip?-1:1);
		float forearmy = army + pose::arm::forearmy[armpose];
		handx = forearmx + pose::forearm::handx[forearmpose]*(forearmflip?-1:1);
		handy = forearmy + pose::forearm::handy[forearmpose];

		 // Now to actually draw.

		draw_image(&img::rata_body, x(), y(), bodypose, flip);
		for (uint i=0; i<MAX_EQUIPS; i++)
		if (equipment[i])
		if (equipment[i]->body)
			draw_image(equipment[i]->body, x(), y(), bodypose, flip);

		if (deadframe) goto draw_arm;
		draw_head:
		draw_image(
			&img::rata_head,
			x() + headx,
			y() + heady,
			headpose, flip
		);
		for (uint i=0; i<MAX_EQUIPS; i++)
		if (equipment[i])
		if (equipment[i]->head)
			draw_image(
				equipment[i]->head,
				x() + headx,
				y() + heady,
				headpose, flip
			);

		if (deadframe) goto draw_hand;
		draw_arm:
		draw_image(
			&img::rata_arm,
			x() + armx,
			y() + army,
			armpose, armflip
		);
		draw_image(
			&img::rata_forearm,
			x() + forearmx,
			y() + forearmy,
			forearmpose, forearmflip
		);
		for (uint i=0; i<MAX_EQUIPS; i++)
		if (equipment[i])
		if (equipment[i]->arm)
			draw_image(
				equipment[i]->arm,
				x() + armx,
				y() + army,
				armpose, armflip
			);
		for (uint i=0; i<MAX_EQUIPS; i++)
		if (equipment[i])
		if (equipment[i]->forearm)
			draw_image(
				equipment[i]->forearm,
				x() + forearmx,
				y() + forearmy,
				forearmpose, forearmflip
			);

		if (deadframe) goto draw_head;
		
		draw_hand:
		for (uint i=0; i<MAX_EQUIPS; i++)
		if (equipment[i])
		if (equipment[i]->hand)
			draw_image(
				equipment[i]->hand,
				x() + handx,
				y() + handy,
				handpose, flip
			);
	}
};



