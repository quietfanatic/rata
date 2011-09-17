#define RATA_STEP 1.0
#define RATA_DRAW(img) draw_image(&img, lx, ly, 0, flip);



struct Rata : Damagable {
	 // Character state
	//Object* floor;
	int float_frames;
	bool aiming;
	bool can_see;
	Object* pointed_object;
	bool take_damage;
	int hurting;
	int flashing;
	float aim_distance;
	float aim_direction;
	float distance_walked;  // For drawing
	float oldxrel;
	int recoil_frames;

	 // Easy access to bits
	float aim_center_x () { return x() + 2*PX*facing; }
	float aim_center_y () { return y() + 13*PX; }
	b2Fixture* fix_main () { return body->GetFixtureList(); }
	b2Fixture* fix_hurt () { return body->GetFixtureList()->GetNext(); }

	b2Fixture* fix_main_current () {
		if (hurting) return fix_hurt();
		else return fix_main();
	}

	void set_fix_normal () {
		fix_main()->SetFilterData(cf::rata);
		fix_hurt()->SetFilterData(cf::disabled);
	}
	void set_fix_invincible () {
		fix_main()->SetFilterData(cf::rata_invincible);
		fix_hurt()->SetFilterData(cf::disabled);
	}
	void set_fix_hurt () {
		fix_main()->SetFilterData(cf::disabled);
		fix_hurt()->SetFilterData(cf::rata_invincible);
	}

	 // Character stats (affected by items and such)
	float ground_accel () {
		return 0.2;
	}
	float ground_decel () {
		return 0.4;
	}
	float max_forward_speed () {
		return 6.0;
	}
	float max_backward_speed () {
		return 4.0;
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
	virtual int max_life () {
		return 144;
	}

	void before_move () {
		//floor = get_floor(fix_feet_current());
		 // Aiming
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
		if (flashing) {
			flashing--;
			if (flashing <= 0) {
				set_fix_normal();
			}
		}
		else if (!hurting && !take_damage) body->GetFixtureList()->SetFilterData(cf::rata);
		if (take_damage) {
			if (floor) add_vel(0, 3.0);
			take_damage = false;
			printf("Ouch!\n");
			set_fix_hurt();
		}
		else if (hurting) {
			hurting--;
			if (hurting <= 0) {
				set_fix_invincible();
				flashing = 60;
			}
		}
		 // Movement
		if (floor && !take_damage) {
			if (hurting < 12) {
				if (hurting > 0) {
					hurting = 0;
					flashing = 60;
					set_fix_invincible();
				}
				oldxrel = x() - floor->x();
				if (key[sf::Key::A] && !key[sf::Key::D]) {  // Left
					float max = facing<=0 ? -max_forward_speed() : -max_backward_speed();
					if (xvelrel(floor) > max) {
						//mutual_impulse(floor, ground_accel()*mass(), 0);
						add_vel(-ground_accel(), 0);
						if (xvelrel(floor) < max) {
							//mutual_impulse(floor, xvelrel(floor) - max, 0);
							add_vel(max - xvelrel(floor), 0);
						}
					}
					else if (facing > 0) goto stop;
				}
				else if (key[sf::Key::D] && !key[sf::Key::A]) {  // Right
					float max = facing>=0 ? max_forward_speed() : max_backward_speed();
					if (xvelrel(floor) < max) {
						//mutual_impulse(floor, -ground_accel()*mass(), 0);
						add_vel(ground_accel(), 0);
						if (xvelrel(floor) > max) {
							//mutual_impulse(floor, xvelrel(floor) - max, 0);
							add_vel(max - xvelrel(floor), 0);
						}
					}
					else if (facing < 0) goto stop;
				}
				else {  // Stop
					stop:
					if (xvelrel(floor) > 0)
						add_vel(-MIN(xvelrel(floor), ground_decel()), 0);
					else if (xvelrel(floor) < 0)
						add_vel(MIN(-xvelrel(floor), ground_decel()), 0);
				}
				 // Jump
				if (key[sf::Key::W] && key[sf::Key::W] == 1) {  // Jump
					//mutual_impulse(floor, 0, -jump_velocity()*mass());
					add_vel(0, jump_velocity());
					float_frames = jump_float_time();
				}
			}
			else {
				 // Do sitting (from hurt) action here.
			}
		}
		else {  // Midair
			if (key[sf::Key::A] && !key[sf::Key::B]) {  // Left
				float max = -max_air_speed();
				if (xvel() > max) {
					add_vel(-air_accel(), 0);
					if (xvel() < max) {
						add_vel(-xvel() + max, 0);
					}
				}
			}
			else if (key[sf::Key::D]) {  // Right
				float max = max_air_speed();
				if (xvel() < max) {
					add_vel(air_accel(), 0);
					if (xvel() > max) {
						add_vel(-xvel() + max, 0);
					}
				}
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
		 // Fire weapon
		if (aiming
		 && recoil_frames == 0
		 && button[sf::Mouse::Left]
		 && !hurting) {
			recoil_frames = 30;
			float bullet_velocity = 120.0;
			(new obj::Desc(obj::bullet, this,
				aim_center_x(),
				aim_center_y(),
				bullet_velocity * cos(aim_direction),
				bullet_velocity * sin(aim_direction),
				0,
				true
			))->manifest();
		}
		else if (recoil_frames) recoil_frames--;
		 // Describe object or advance message
		if (!aiming) {
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
	};

	virtual void damage (int power) {
		if (!hurting && !flashing) {
			Damagable::damage(power);
			take_damage = true;
			hurting = 6 + power / 2;
		}
	};




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

	static obj::Desc lifebar_desc;

	virtual void on_create () {
		make_body(desc, true, true);
			for (uint i = obj::def[desc->id].nfixes; i > 0; i--) {
				dbg(4, "Fix %d: 0x%08x\n", i, body->CreateFixture(&(obj::def[desc->id].fixdef[i-1])));
			}
		dbg(3, "Affixed 0x%08x with 0x%08x\n", this, body);
		floor = NULL;
		float_frames = 0;
		recoil_frames = 0;
		take_damage = false;
		hurting = 0;
		flashing = 0;
		life = max_life();
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
		lifebar_desc.manifest();
	}
	virtual void on_destroy () {
		rata = NULL;
	}

	void draw () {
		if (flashing % 3 == 2) return;
		//sf::Sprite s;
		//s.SetCenter(32,34);
		float lx = x();
		float ly = y();
		bool flip = facing<0;
		//move_sprite(&s);
		 // Flip if facing left
		//if (facing < 0) {
			//s.FlipX(true);
			//s.SetCenter(30,34);
		//}
		if (hurting) {
			RATA_DRAW(img::body_dress_hurtback);
			RATA_DRAW(img::arm_dress_hurtback);
			RATA_DRAW(img::head_hurtback);
			RATA_DRAW(img::handgun_hurtback);
			return;
		}
		 // Select body pose
		int walk_pose;
		if (floor) {
			float step_d = MOD(distance_walked, RATA_STEP * 2);
			if (step_d < 0.01) walk_pose = 0;
			else if (step_d < RATA_STEP*5/9.0) walk_pose = 1;
			else if (step_d < RATA_STEP) walk_pose = 0;
			else if (step_d < RATA_STEP*14/9.0) walk_pose = -1;
			else walk_pose = 0;
		}
		else walk_pose = 1;
		if (walk_pose != 0) {
				//s.SetY(s.GetPosition().y+window_scale);
				ly -= 1*PX;
				RATA_DRAW(img::body_dress_walk);
		}
		else {
			RATA_DRAW(img::body_dress_stand);
		}
		 // Select aim angle
		//printf("%f -> ", aim_direction);
		float aim_angle;
		if (facing > 0)
			aim_angle = aim_direction;
		else if (aim_direction > 0)
			aim_angle = M_PI - aim_direction;
		else
			aim_angle = -M_PI - aim_direction;
		if (recoil_frames >= 20) aim_angle += M_PI/8.0;
		//printf("%f\n", aim_angle);
		if (!aiming) {
			if (walk_pose == 1) {
				RATA_DRAW(img::arm_dress_walk_f);
				RATA_DRAW(img::handgun_walk_f);
			}
			else if (walk_pose == -1) {
				RATA_DRAW(img::arm_dress_walk_b);
				RATA_DRAW(img::handgun_walk_b);
			}
			else {
				RATA_DRAW(img::arm_dress_stand);
				RATA_DRAW(img::handgun_stand);
			}
		}
		bool walk_head = floor ? (walk_pose != 0) : (yvel() < -1.0);
		if (aim_angle > M_PI*5.0/16.0) {
			if (aiming) { RATA_DRAW(img::arm_dress_m158); }
			if (walk_head) { RATA_DRAW(img::head_walk_158); }
			else { RATA_DRAW(img::head_stand_158); }
			if (aiming) { RATA_DRAW(img::handgun_m158); }
		}
		else if (aim_angle > M_PI*3.0/16.0) {
			if (aiming) { RATA_DRAW(img::arm_dress_m135); }
			if (walk_head) { RATA_DRAW(img::head_walk_135); }
			else { RATA_DRAW(img::head_stand_135); }
			if (aiming) { RATA_DRAW(img::handgun_m135); }
		}
		else if (aim_angle > M_PI*1.0/16.0) {
			if (aiming) { RATA_DRAW(img::arm_dress_m113); }
			if (walk_head) { RATA_DRAW(img::head_walk_90); }
			else { RATA_DRAW(img::head_stand_90); }
			if (aiming) { RATA_DRAW(img::handgun_m113); }
		}
		else if (aim_angle > M_PI*-1.0/16.0) {
			if (aiming) { RATA_DRAW(img::arm_dress_m90); }
			if (walk_head) { RATA_DRAW(img::head_walk_90); }
			else { RATA_DRAW(img::head_stand_90); }
			if (aiming) { RATA_DRAW(img::handgun_m90); }
		}
		else if (aim_angle > M_PI*-3.0/16.0) {
			if (aiming) { RATA_DRAW(img::arm_dress_m68); }
			if (walk_head) { RATA_DRAW(img::head_walk_68); }
			else { RATA_DRAW(img::head_stand_68); }
			if (aiming) { RATA_DRAW(img::handgun_m68); }
		}
		else if (aim_angle > M_PI*-5.0/16.0) {
			if (aiming) { RATA_DRAW(img::arm_dress_m45); }
			if (walk_head) { RATA_DRAW(img::head_walk_45); }
			else { RATA_DRAW(img::head_stand_45); }
			if (aiming) { RATA_DRAW(img::handgun_m45); }
		}
		else {
			if (aiming) { RATA_DRAW(img::arm_dress_m23); }
			if (walk_head) { RATA_DRAW(img::head_walk_23); }
			else { RATA_DRAW(img::head_stand_23); }
			if (aiming) { RATA_DRAW(img::handgun_m23); }
		}
	}
};
obj::Desc Rata::lifebar_desc = obj::Desc(obj::lifebar);




