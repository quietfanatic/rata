#define MAX_INVENTORY 10


struct Rata : Walking {
	 // Character state
	enum State {
		standing = 1,
		walking,
		kneeling,
		crawling,
		falling,
		ouch,
		hurt_air,
		hurt,
		dead_air,
		dead
	};
	uint state;
	 // Timers
	int float_frames;
	int recoil_frames;
	int hurt_frames;
	int inv_frames;
	int adrenaline;
	int hurt_direction;
	int hurt_id [2];
	float min_aim;
	float max_aim;
	 // Aiming
	bool aiming;
	bool can_see;
	Object* pointed_object;
	float aim_distance;
	float aim_direction;
	 // For animation
	float distance_walked;
	float oldxrel;
	Vec hand_pos;
	uint angle_frame;  // 0=down, 8=up
	float oldyvel;
	float helmet_angle;
	 // Fixtures
	b2Fixture* fix_feet;
	b2Fixture* fix_current;
	b2Fixture* fix_27;
	b2Fixture* fix_25;
	b2Fixture* fix_21;
	b2Fixture* fix_h7;
	b2Fixture* fix_crawl_r;
	b2Fixture* fix_crawl_l;
	b2Fixture* fix_sensor_21;
	b2Fixture* fix_sensor_floor_r;
	b2Fixture* fix_sensor_floor_l;
	b2Fixture* fix_helmet_current;
	b2Fixture* fix_helmet_stand;
	b2Fixture* fix_helmet_kneel;
	b2Fixture* fix_helmet_crawl_r;
	b2Fixture* fix_helmet_crawl_l;
	 // Equipment
	uint inventory_amount;
	obj::Desc* inventory [10];
	obj::Desc* equipment [item::num_slots];
	 // Actions
	float action_distance;
	void* action_arg;
	enum Action {
		action_equip,
		action_enter,
	};
	int action;
	void propose_action (int act, void* arg, Vec p, float radius) {
		if (y() > p.y - radius)
		if (y() < p.y + radius)
		if (x() > p.x - radius)
		if (x() < p.x + radius) {
			float dist = abs_f(x() - p.x);
			if (dist < action_distance || dist < 0.2) {
				action = act;
				action_arg = arg;
				action_distance = dist;
			}
		}
	}

	 // Easy access to bits
	item::Def* equip_info (uint i) {
		return equipment[i] ? &item::def[equipment[i]->data] : NULL;
	}
	bool wearing_helmet () {
		return equip_info(item::head) == &item::def[item::helmet];
	}
	Vec aim_center () { return pos() + Vec(2*PX*facing, 13*PX); }

//	b2Fixture* fix_current () {
//		if (hurt_frames) return fix_27();
//		else if (kneeling) return fix_21();
//		else return fix_25();
//	}
	bool check_fix (b2Fixture* fix) {
		for (b2ContactEdge* ce = body->GetContactList(); ce; ce=ce->next) {
			b2Contact* c = ce->contact;
			if (c->GetFixtureA() == fix || c->GetFixtureB() == fix)
			if (c->IsTouching() && c->IsEnabled())
				return true;
		}
		return false;
	}
	bool bullet_inv () {
		return state == dead
		    || state == dead_air
		    || hurt_id[0] == obj::bullet
		    || hurt_id[1] == obj::bullet;
	}
	void set_fix (b2Fixture* fix) {
		fix_feet->SetFilterData(bullet_inv() ? cf::rata_invincible : cf::rata);
		if (fix_current && fix_current != fix) {
			fix_current->SetFilterData(cf::disabled);
		}
		fix_current = fix;
		fix->SetFilterData(bullet_inv() ? cf::rata_invincible : cf::rata);
	}
	void set_helmet (b2Fixture* fix) {
		if (fix && wearing_helmet()) {
			if (fix_helmet_current && fix_helmet_current != fix) {
				fix_helmet_current->SetFilterData(cf::disabled);
			}
			fix->SetFilterData(bullet_inv() ? cf::rata_invincible : cf::rata);
		}
		else if (fix_helmet_current) {
			fix_helmet_current->SetFilterData(cf::disabled);
		}
		fix_helmet_current = fix;
	}


	 // Equipment and inventory management

	void spawn_item (obj::Desc* itemdesc) {
		itemdesc->room = room::currenti;
		itemdesc->pos = pos();
		itemdesc->facing = facing;
		itemdesc->manifest();
	}
	void add_to_inventory (obj::Desc* itemdesc) {
		for (uint i=0; i < MAX_INVENTORY; i++) {
			if (inventory[i] == 0) {
				inventory[i] = itemdesc;
				inventory_amount++;
				return;
			}
		} // No room
		spawn_item(itemdesc);
	}	
	void pick_up (Item* itemobj) {
		itemobj->destroy();
		itemobj->desc->room = room::inventory;
		add_to_inventory(itemobj->desc);
	}
	void drop (uint i) {
		spawn_item(inventory[i]);
		for (; i < MAX_INVENTORY - 1; i++) {
			inventory[i] = inventory[i+1];
		}
		inventory[MAX_INVENTORY] = NULL;
	}
	void unequip_drop (obj::Desc* itemdesc) {
		if (itemdesc == NULL) return;
		int slot = item::def[itemdesc->data].slot;
		equipment[slot] = NULL;
		int otherslot = item::def[itemdesc->data].otherslot;
		if (otherslot > 0) equipment[otherslot] = NULL;
		spawn_item(itemdesc);
	}
	void pick_up_equip (Item* itemobj) {
		itemobj->destroy();
		itemobj->desc->room = room::inventory;
		int slot = item::def[itemobj->desc->data].slot;
		if (equipment[slot])
			unequip_drop(equipment[slot]);
		int otherslot = item::def[itemobj->desc->data].otherslot;
		if (otherslot > 0 && equipment[otherslot])
			unequip_drop(equipment[otherslot]);
		equipment[slot] = itemobj->desc;
		if (otherslot > 0) equipment[otherslot] = itemobj->desc;
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



	 // Actions that can be taken
	void allow_turn () {
		if (cursor.x > 0) facing = 1;
		else if (cursor.x < 0) facing = -1;
	}
	void allow_look () {
		aim_distance = sqrt(cursor.x*cursor.x + cursor.y*cursor.y);
		aim_direction = atan2(cursor.y, cursor.x);
		pointed_object = check_area(
			aim_center().x + cursor.x + 1*PX, aim_center().y + cursor.y + 1*PX,
			aim_center().x + cursor.x - 1*PX, aim_center().y + cursor.y - 1*PX,
			1|2|4|8|16|32|64
		);
	}
	bool allow_aim () {
		aiming = (button[sf::Mouse::Right] > 0 || key[sf::Key::LShift] > 0);
		if (aiming) {
			if (facing > 0) {
				if (aim_direction > max_aim)
					aim_direction = max_aim;
				else if (aim_direction < min_aim)
					aim_direction = min_aim;
			}
			else {
				if (flip_angle(aim_direction) > max_aim)
					aim_direction = flip_angle(max_aim);
				else if (flip_angle(aim_direction) < min_aim)
					aim_direction = flip_angle(min_aim);
			}
		}
		return aiming;
	}

	bool allow_walk () {
		 // Left
		if (key[sf::Key::A] && !key[sf::Key::D]) {
			if (xvel() < 0)
				floor_friction = ground_accel();
			else floor_friction = ground_decel();
			if (facing < 0)
				ideal_xvel = !aiming ? -max_forward_speed() : -max_backward_speed();
			else
				ideal_xvel = -max_backward_speed();
		}
		 // Right
		else if (key[sf::Key::D] && !key[sf::Key::A]) {
			if (xvel() > 0)
				floor_friction = ground_accel();
			else floor_friction = ground_decel();
			if (facing > 0)
				ideal_xvel = !aiming ? max_forward_speed() : max_backward_speed();
			else
				ideal_xvel = max_backward_speed();
		}
		 // Stop
		else {
			floor_friction = ground_decel();
			ideal_xvel = 0;
		}
		return abs_f(xvel()) >= 0.01;
	}

	bool allow_jump () {
		if (key[sf::Key::W] && key[sf::Key::W] == 1) {  // Jump
			//mutual_impulse(floor, 0, -jump_velocity()*mass());
			set_vel(Vec(vel().x, jump_velocity()));
			float_frames = jump_float_time();
			state = falling;
			return true;
		}
		else return false;
	}

	bool allow_kneel () {
		return (key[sf::Key::S] && floor_normal.y > 0.9)
		    || (state == crawling && check_fix(fix_sensor_21));
	}

	bool allow_crawl () {
		floor_friction = ground_decel()*ground_decel();
		if (key[sf::Key::A] && !key[sf::Key::D]) {
			if (!crawling) facing = -1;
			ideal_xvel = -max_crawl_speed();
			return true;
		}
		else if (key[sf::Key::D] && !key[sf::Key::A]) {
			if (!crawling) facing = 1;
			ideal_xvel = max_crawl_speed();
			return true;
		}
		else {
			ideal_xvel = 0;
			return false;
		};
	}
	
	void allow_airmove () {
		 // Left
		if (key[sf::Key::A] && !key[sf::Key::B]) {
			float max = -max_air_speed();
			if (xvel() > max) {
				add_vel(Vec(-air_accel(), 0));
				if (xvel() < max) {
					add_vel(Vec(-xvel() + max, 0));
				}
			}
		}
		 // Right
		else if (key[sf::Key::D]) {
			float max = max_air_speed();
			if (xvel() < max) {
				add_vel(Vec(air_accel(), 0));
				if (xvel() > max) {
					add_vel(Vec(-xvel() + max, 0));
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
	}

	void allow_use () {
		if (aiming
		 && recoil_frames == 0
		 && button[sf::Mouse::Left]) {
			if (equip_info(item::hand))
			if (equip_info(item::hand)->use) {
				(*equip_info(item::hand)->use)();
			}
		}
	}

	void allow_action () {
		if (key[sf::Key::Space] == 1)
		switch (action) {
			case action_equip: {
				pick_up_equip((Item*)action_arg);
				break;
			}
			case action_enter: {
				obj::Desc* d = ((Object*)action_arg)->desc;
				room::list[d->data]->enter(d->data2);
				break;
			}
			default: { }
		}
	}

	void allow_examine () {
		if (!button[sf::Mouse::Right] && !key[sf::Key::LShift]) {
			if (abs_f(cursor.x) < 0.1 && abs_f(cursor.y) < 0.1) {
				can_see = true;
			}
			else if (cursor.x*facing < 0) {
				can_see = false;
			}
			else {
				b2Fixture* seeing = check_line(
					aim_center(), aim_center() + Vec(cursor.x, cursor.y)
				).hit;
				can_see = (seeing == NULL);
			}
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
	}

	void decrement_counters () {
		if (recoil_frames) recoil_frames--;
		if (yvel() < -20.0) {
			hurt_frames++;
			state = hurt_air;
		}
		if (hurt_frames) {
			if (yvel() < -10.0)
				hurt_frames++;
			else
				hurt_frames--;
		}
		if (hurt_frames > 120) hurt_frames = 120;
		if (inv_frames) inv_frames--;
		else hurt_id[0] = hurt_id[1] = -1;
		if (adrenaline) adrenaline--;
	}

	void before_move () {
		aiming = false;
		switch (state) {
			case standing:
			case walking:
			case kneeling:
			case crawling: {
				if (!floor) goto no_floor;
				got_floor:
				if (allow_kneel()) {
					if (allow_crawl()) {
						state = crawling;
						if (!check_fix(fix_sensor_21)) {
							allow_turn();
						}
						allow_look();
						if (facing > 0)
							set_fix(fix_crawl_r),
							set_helmet(fix_helmet_crawl_r);
						else
							set_fix(fix_crawl_l),
							set_helmet(fix_helmet_crawl_l);
					}
					else if (state == crawling) {
						if (check_fix(fix_sensor_21)) {
							allow_look();
							if (check_fix(facing>0 ? fix_sensor_floor_r : fix_sensor_floor_l))
								min_aim = -1*M_PI/17;
							else
								min_aim = -3*M_PI/8;
							max_aim = 1*M_PI/8;
							allow_aim();
							allow_use();
							if (facing > 0) {
								set_fix(fix_crawl_r);
								set_helmet(fix_helmet_crawl_r);
							}
							else {
								set_fix(fix_crawl_l);
								set_helmet(fix_helmet_crawl_l);
							}
						}
						else {
							allow_turn();
							allow_look();
							if (check_fix(facing>0 ? fix_sensor_floor_r : fix_sensor_floor_l))
								min_aim = -1*M_PI/17;
							else
								min_aim = -3*M_PI/8;
							max_aim = wearing_helmet() ? 3*M_PI/8 : M_PI/2;
							if (allow_aim()) {
								float aim = facing>0 ? aim_direction : flip_angle(aim_direction);
								if (aim > 1*M_PI/8) goto kneel;
								else allow_use();
							}
							if (facing > 0) {
								set_fix(fix_crawl_r);
								set_helmet(fix_helmet_crawl_r);
							}
							else {
								set_fix(fix_crawl_l);
								set_helmet(fix_helmet_crawl_l);
							}
						}
					}
					else {
						allow_turn();
						allow_look();
						if (check_fix(facing>0 ? fix_sensor_floor_r : fix_sensor_floor_l))
							min_aim = -1*M_PI/4;
						else
							min_aim = -3*M_PI/8;
						max_aim = wearing_helmet() ? 3*M_PI/8 : M_PI/2;
						allow_aim();
						kneel:
						state = kneeling;
						floor_friction = ground_decel();
						ideal_xvel = 0;
						allow_use();
						set_fix(fix_21);
						set_helmet(fix_helmet_kneel);
					}
				}
				else if (allow_jump()) {
					state = falling;
					allow_turn();
					allow_look();
					min_aim = -M_PI;
					max_aim = wearing_helmet() ? 3*M_PI/8 : M_PI/2;
					allow_aim();
					allow_airmove();
					set_fix(fix_27);
					set_helmet(fix_helmet_stand);
				}
				else {
					allow_turn();
					allow_look();
					min_aim = -3*M_PI/8;
					max_aim = wearing_helmet() ? 3*M_PI/8 : M_PI/2;
					allow_aim();
					if (allow_walk()) {
						state = walking;
						allow_use();
					}
					else {
						state = standing;
						allow_use();
					}
					set_fix(fix_27);
					set_helmet(fix_helmet_stand);
				}
				allow_action();
				allow_examine();
				decrement_counters();
				break;
			}
			case falling: {
				if (floor) {
					body->SetGravityScale(1.0);
					float_frames = 0;
					snd::step.play(1.2, 8*-oldyvel);
					goto got_floor;
				}
				no_floor:
				state = falling;
				allow_turn();
				allow_look();
				min_aim = -M_PI;
				max_aim = wearing_helmet() ? 3*M_PI/8 : M_PI/2;
				allow_aim();
				allow_airmove();
				allow_use();
				allow_action();
				allow_examine();
				decrement_counters();
				set_fix(fix_27);
				set_helmet(fix_helmet_stand);
				break;
			}
			case ouch: {
				body->SetGravityScale(1.0);
				float_frames = 0;
				if (yvel() < 1.0) add_vel(Vec(0, 3.0));
				if (life <= 0) {
					impulse(Vec(0, 5.0));
					goto dead_no_floor;
				}
				else {
					goto hurt_no_floor;
				}
				break;
			}
			case hurt_air: {
				hurt_direction = sign_f(xvel())*facing;
				if (floor) {
					if (hurt_frames < 20) {
						snd::step.play(1.2, 8*-oldyvel/2.0);
						goto got_floor;
					}
					else {
						snd::fall.play(0.9, 6*-oldyvel/2.0);
						goto hurt_floor;
					}
				}
				hurt_no_floor:
				if (hurt_frames == 0) goto no_floor;
				state = hurt_air;
				allow_look();
				allow_examine();
				decrement_counters();
				set_fix(fix_25);
				set_helmet(fix_helmet_stand);
				break;
			}
			case hurt: {
				if (!floor) goto hurt_no_floor;
				if (hurt_frames == 0) goto got_floor;
				hurt_floor:
				state = hurt;
				floor_friction = ground_decel();
				ideal_xvel = 0;
				allow_look();
				allow_examine();
				decrement_counters();
				set_fix(fix_21);
				set_helmet(fix_helmet_kneel);
				break;
			}
			case dead_air: {
				if (floor && floor_normal.y > 0.9) {
					snd::fall.play(0.9, 6*-oldyvel/2.0);
					goto dead_floor;
				}
				dead_no_floor:
				state = dead_air;
				set_fix(fix_25);
				set_helmet(fix_helmet_stand);
				break;
			}
			case dead: {
				if (!floor) goto dead_no_floor;
				dead_floor:
				state = dead;
				floor_friction = ground_decel();
				ideal_xvel = 0;
				set_fix(fix_h7);
				set_helmet(NULL);
				break;
			}	
		}

		if (floor) oldxrel = x() - floor->x();
		oldyvel = yvel();

		 // Select cursor image
		if (aiming) cursor.img = &img::target;
		else if (message) cursor.img = &img::readmore;
		else if (can_see) {
			if (pointed_object && pointed_object->desc->id != obj::tilemap)
				cursor.img = &img::see;
			else cursor.img = &img::look;
		}
		else cursor.img = &img::nolook;

		Walking::before_move();
	}

	virtual void damage (int d) {
		Object::damage(d);
		state = ouch;
		hurt_frames = (20 + d) / (2 + adrenaline/60.0);
		inv_frames = 50;
		adrenaline += 5*d;
		//printf("Hurt status: %d, %d, %d\n", hurt_frames, inv_frames, adrenaline);
		snd::hurt.play(1.0 - 0.1*d/24, 40 + d);
	}

	virtual void kill () {
		trap_cursor = false;
		message = message_pos = message_pos_next = NULL;
	}




	void after_move () {
//		printf("%08x's floor is: %08x\n", this, floor);
//		floor = get_floor(fix_feet_current());
		float step = state == crawling ? 0.8 : 1.0;
		if (floor && (state == walking || state == crawling)) {
			if (abs_f(xvel()) < 0.01)
				distance_walked = 0;
			else {
				float olddist = distance_walked;
				distance_walked += ((x() - floor->x()) - oldxrel)*sign_f(xvel());
				if (state == walking) {
					float oldstep = mod_f(olddist, step);
					float step_d = mod_f(distance_walked, step);
					if (oldstep < 0.4 && step_d >= 0.4)
						snd::step.play(0.9+rand()*0.2/RAND_MAX, 6*abs_f(xvel())*(1.0+rand()*0.2/RAND_MAX));
				}
			}
		}
		else distance_walked = 0;
	};
	virtual void on_create () {
		make_body(desc, true, true);
			for (uint i = obj::def[desc->id].nfixes; i > 0; i--) {
				dbg(4, "Fix %d: 0x%08x\n", i, body->CreateFixture(&(obj::def[desc->id].fixdef[i-1])));
			}
		fix_feet = body->GetFixtureList();
		fix_27 = fix_feet->GetNext();
		fix_25 = fix_27->GetNext();
		fix_21 = fix_25->GetNext();
		fix_h7 = fix_21->GetNext();
		fix_crawl_r = fix_h7->GetNext();
		fix_crawl_l = fix_crawl_r->GetNext();
		fix_sensor_21 = fix_crawl_l->GetNext();
		fix_sensor_floor_r = fix_sensor_21->GetNext();
		fix_sensor_floor_l = fix_sensor_floor_r->GetNext();
		fix_helmet_stand = fix_sensor_floor_l->GetNext();
		fix_helmet_kneel = fix_helmet_stand->GetNext();
		fix_helmet_crawl_r = fix_helmet_kneel->GetNext();
		fix_helmet_crawl_l = fix_helmet_crawl_r->GetNext();
		fix_current = fix_helmet_current = NULL;
		set_fix(fix_27);
		dbg(3, "Affixed 0x%08x with 0x%08x\n", this, body);
		floor = NULL;
		Walking::on_create();
		float_frames = 0;
		recoil_frames = 0;
		hurt_frames = 0;
		inv_frames = 0;
		adrenaline = 0;
		hurt_id[0] = hurt_id[1] = -1;
		if (desc->data) state = desc->data;
		else state = falling;
		life = max_life = 144;
		inventory_amount = 0;
		for (uint i=0; i<item::num_slots; i++) equipment[i] = NULL;
		for (uint i=0; i<MAX_INVENTORY; i++) inventory[i] = NULL;
		equipment[item::body] = new obj::Desc(-2, obj::item, Vec(0, 0), Vec(0, 0), 0, item::white_dress);
		facing = desc->facing ? desc->facing : 1;
		cursor.x = 2.0 * facing;
		cursor.y = 0;
		cursor.img = &img::look;
		trap_cursor = true;
		oldyvel = 0.0;
		rata = this;
	}
	virtual void on_destroy () {
		rata = NULL;
	}

	void draw () {

		float step = state == crawling ? 0.8 : 1.0;
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
		Vec p = pos();
		if (equip_info(item::feet))
		if (state == standing || state == walking) p.y += 1*PX;
		
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
		: aim_angle > M_PI*-5.0/16.0 ? 2
		: aim_angle > M_PI*-7.0/16.0 ? 1
		:                              0;

		 // Select walking frame
		if (floor) {
			float step_d = mod_f(distance_walked, step * 2);
			if (step_d < 0) step_d += step*2;
			walk_frame =
			  abs_f(xvel()) < 0.01 ? 4
			: step_d < step*5/9.0  ? 1
			: step_d < step        ? 2
			: step_d < step*14/9.0 ? 3
			:                        4;
		}
		else walk_frame = 3;

		 // Select body pose
		{ using namespace pose::body;
			bodypose =
			  state == standing ? stand
			: state == walking  ? (walk_frame % 2) ? walk
			                    :                    stand
			: state == kneeling ? kneel
			: state == crawling ? (walk_frame % 2) ? crawl2
			                    :                    crawl
			: state == falling  ? walk
			: state == hurt_air ? hurtbk
			: state == hurt     ? hurt_direction == 1 ? crawl : sit
			: state == dead_air ? hurtbk
			: state == dead     ? laybk
			:                     walk;
		}

		 // Select head pose
		{ using namespace pose::head;
			headpose =
			  state == standing ? angle_stand[angle_frame]
			: state == walking  ? (walk_frame % 2) ? angle_walk[angle_frame]
			                    :                    angle_stand[angle_frame]
			: state == kneeling ? angle_stand[angle_frame]
			: state == crawling ? crawl
			: state == falling  ? (yvel() < 0) ? angle_walk[angle_frame]
			                    :                angle_stand[angle_frame]
			: state == hurt_air ? hurtbk
			: state == hurt     ? hurt_direction == 1 ? crawl : hurtbk
			: state == dead_air ? hurtbk
			: state == dead     ? laybk
			:                     stand_90;
			helmet_angle = helmeta[headpose];
		}

		 // Select arm pose
		{ using namespace pose::arm;
			armpose =
			  aiming && state == crawling ? a23 
			: aiming ? recoil_frames > 20 ? angle_frame - 3
			         : aim_distance > 10  ? angle_frame
			         : aim_distance > 4   ? angle_frame == 0 ? 0
			                              :                    angle_frame - 2
			         :                      angle_frame == 0 ? 0
			                              : angle_frame == 1 ? -1
			                              :                    angle_frame - 4
			: state == standing ? a0
			: state == walking  ? walk_frame == 1 ? -a23
			                    : walk_frame == 2 ? a0
			                    : walk_frame == 3 ? a23
			                    :                   a0
			: state == kneeling ? a45
			: state == crawling ? walk_frame == 1 ? a45
			                    : walk_frame == 2 ? a0
			                    : walk_frame == 3 ? -a45
			                    :                   a0
			: state == falling  ? a23
			: state == hurt_air ? a23
			: state == hurt     ? hurt_direction == 1 ? a0 : -a23
			: state == dead_air ? a23
			: state == dead     ? a90
			:                     a0;
			if (armpose < a0) armpose = -armpose, armflip = !armflip;
			if (armpose > a180) armpose = (2*a180)-armpose, armflip = !armflip;
		}

		 // Select forearm pose
		{ using namespace pose::forearm;
			forearmpose =
			  aiming ? recoil_frames > 20 ? angle_frame + 1
			         :                      angle_frame
			: state == standing ? a0
			: state == walking  ? walk_frame == 1 ? -a23
			                    : walk_frame == 2 ? a0
			                    : walk_frame == 3 ? a23
			                    :                   a0
			: state == kneeling ? a68
			: state == crawling ? a90
			: state == falling  ? a23
			: state == hurt_air ? a68
			: state == hurt     ? hurt_direction == 1 ? a90 : -a23
			: state == dead_air ? a68
			: state == dead     ? a90
			:                     a0;
			if (forearmpose < a0) forearmpose = -forearmpose, forearmflip = !forearmflip;
			if (forearmpose > a180) forearmpose = (2*a180)-forearmpose, forearmflip = !forearmflip;
		}

		 // Select hand pose
		{ using namespace pose::hand;
			handpose =
			  aiming ? recoil_frames > 20 ? angle_frame + 1
			         :                      angle_frame
			: state == standing ? a0
			: state == walking  ? walk_frame == 1 ? a338
			                    : walk_frame == 2 ? a0
			                    : walk_frame == 3 ? a23
			                    :                   a0
			: state == kneeling ? a45
			: state == crawling ? inside
			: state == falling  ? a23
			: state == hurt_air ? a68
			: state == hurt     ? hurt_direction == 1 ? inside : front
			: state == dead_air ? a68
			: state == dead     ? inside
			:                     a0;
		}
		
		 // Get position of various parts.
		
		Vec headp = Vec(pose::body::headx[bodypose]*facing,
		                  pose::body::heady[bodypose]);
		Vec helmetp = headp + Vec(pose::head::helmetx[headpose] * facing,
		                          pose::head::helmety[headpose]);
		Vec armp = Vec(pose::body::armx[bodypose]*facing,
		               pose::body::army[bodypose]);
		Vec forearmp = armp + Vec(pose::arm::forearmx[armpose]*(armflip?-1:1),
		                          pose::arm::forearmy[armpose]);
		hand_pos = forearmp + Vec(pose::forearm::handx[forearmpose]*(forearmflip?-1:1),
		                          pose::forearm::handy[forearmpose]);

		 // Now to actually draw.

		draw_image(&img::rata_body, p, bodypose, flip);
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->body)
			draw_image(equip_info(i)->body, p, bodypose, flip);

		if (state == dead) goto draw_arm;
		draw_head:
		draw_image(
			&img::rata_head,
			p + headp,
			headpose, flip
		);
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->head)
			draw_image(
				equip_info(i)->head,
				p + headp,
				headpose, flip
			);
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->helmet)
			draw_image(
				equip_info(i)->helmet,
				p + helmetp,
				pose::head::helmetf[headpose], flip
			);

		if (state == dead) goto draw_hand;
		draw_arm:
		draw_image(
			&img::rata_arm,
			p + armp,
			armpose, armflip
		);
		draw_image(
			&img::rata_forearm,
			p + forearmp,
			forearmpose, forearmflip
		);
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->arm)
			draw_image(
				equip_info(i)->arm,
				p + armp,
				armpose, armflip
			);
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->forearm)
			draw_image(
				equip_info(i)->forearm,
				p + forearmp,
				forearmpose, forearmflip
			);

		if (state == dead) goto draw_head;
		
		draw_hand:
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->hand)
			draw_image(
				equip_info(i)->hand,
				p + hand_pos,
				handpose, flip
			);
		
		 // Draw action message
		const char* m =
		  action == action_equip ? "EQUIP"
		: action == action_enter ? "ENTER"
		:                          NULL;
		if (m) {
			float w = text_width_small((char*)m)*PX;
			float mx = p.x - w/2;
			float my = p.y + 3;
			draw_rect(mx-2*PX, my, mx + w+1*PX, my - 7*PX, sf::Color(31, 31, 31, 127));
			render_text((char*)m, mx, my, 1, false, true);
		}
		action = -1;
		action_distance = 10000000;

	}
};



