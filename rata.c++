#define MAX_INVENTORY 10


static const uint n_sight_points = 3;
const Vec sight_points_stand [n_sight_points] = {Vec(0, 13*PX), Vec(0, 25*PX), Vec(0, 2*PX)};
const Vec sight_points_kneel [n_sight_points] = {Vec(0, 10*PX), Vec(0, 19*PX), Vec(0, 2*PX)};
const Vec sight_points_crawl_r [n_sight_points] = {Vec(0, 6*PX), Vec(7*PX, 13*PX), Vec(-8*PX, 2*PX)};
const Vec sight_points_crawl_l [n_sight_points] = {Vec(0, 6*PX), Vec(-7*PX, 13*PX), Vec(8*PX, 2*PX)};


MoveStats default_stats = {
	1.0,  // decel
	1.0, 4.0,  // walk
	0.5, 6.0,  // run
	1.0, 2.0,  // crawl
	0.2, 4.0,  // air
	10.0, 1.0,  // jumping stats
};



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
	struct Pose {
		int8 body;
		int8 head;
		int8 helmet;
		int8 arm;
		int8 forearm;
		int8 hand;
	} pose;
	 // Control
	bool auto_control;
	map::Pos destination;
	bool control_left;
	bool control_right;
	bool control_jump;
	bool control_kneel;
	bool control_action;
	bool control_aim;
	bool control_click;
	bool control_goto;
	 // Timers
	int float_frames;
	int recoil_frames;
	int hurt_frames;
	int inv_frames;
	int adrenaline;
	int hurt_direction;
	int hurt_id [2];
	 // Aiming
	float min_aim;
	float max_aim;
	bool aiming;
	bool can_see;
	Object* pointed_object;
	float aim_distance;
	float aim_direction;
	 // For animation and movement
	float distance_walked;
	float oldxrel;
	Vec hand_pos;
	uint angle_frame;  // 0=down, 8=up
	float oldyvel;
	float helmet_angle;
	const Vec* sight_points;
	 // Fixtures
	enum {
		fix_feet,
		fix_27,
		fix_25,
		fix_21,
		fix_h7,
		fix_crawl_r,
		fix_crawl_l,
		fix_sensor_21,
		fix_sensor_floor_r,
		fix_sensor_floor_l,
		fix_sensor_block_r,
		fix_sensor_block_l,
		fix_sensor_wall_r,
		fix_sensor_wall_l,
		fix_helmet_stand,
		fix_helmet_kneel,
		fix_helmet_crawl_r,
		fix_helmet_crawl_l,
	};
	int fix_current;
	int fix_old;
	int fix_helmet_current;
	int fix_helmet_old;
	 // Equipment and inventory
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
	Vec cursor_pos () { return aim_center() + Vec(cursor.x, cursor.y); }
	
	void set_fix (int fix) {
		fix_current = fix;
	}
	void set_helmet (int fix) {
		fix_helmet_current = fix;
	}

	bool check_fix (int fixi) {
		b2Fixture* fix = body->GetFixtureList();
		for (int i=0; i < fixi; i++)
			fix = fix->GetNext();
		for (b2ContactEdge* ce = body->GetContactList(); ce; ce=ce->next) {
			b2Contact* c = ce->contact;
			if (c->GetFixtureA() == fix || c->GetFixtureB() == fix)
			if (c->IsTouching() && c->IsEnabled())
				return true;
		}
		return false;
	}
	bool check_sensor_floor () {
		return (facing > 0) ? check_fix(fix_sensor_floor_r) : check_fix(fix_sensor_floor_l);
	}
	bool check_sensor_block () {
		return (facing > 0) ? check_fix(fix_sensor_block_r) : check_fix(fix_sensor_block_l);
	}
	bool check_sensor_wall () {
		return (facing > 0) ? check_fix(fix_sensor_wall_r) : check_fix(fix_sensor_wall_l);
	}
	bool bullet_inv () {
		return state == dead
		    || state == dead_air
		    || hurt_id[0] == obj::bullet
		    || hurt_id[1] == obj::bullet;
	}
	void update_fixtures () {
		int i = 0;
		for (b2Fixture* fix = body->GetFixtureList(); fix; fix = fix->GetNext()) {
			if (i == fix_current || i == fix_helmet_current || i == fix_feet) {
				fix->SetFilterData(bullet_inv() ? cf::rata_invincible : cf::rata);
			}
			else if (i == fix_old || i == fix_helmet_old) {
				fix->SetFilterData(cf::disabled);
			}
			i++;
		}
	}


	 // Equipment and inventory management

	void spawn_item (obj::Desc* itemdesc) {
		itemdesc->room = room::current;
		itemdesc->pos = pos();
		itemdesc->facing = facing;
		itemdesc->manifest();
	}
	void add_to_inventory (obj::Desc* itemdesc) {
		for (uint i=0; i < MAX_INVENTORY; i++) {
			if (inventory[i] == NULL) {
				inventory[i] = itemdesc;
				itemdesc->room = -100 - i;
				return;
			}
		} // No room
		spawn_item(itemdesc);
	}	
	void pick_up (Item* itemobj) {
		itemobj->destroy();
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
		int slot = item::def[itemobj->desc->data].slot;
		if (equipment[slot])
			unequip_drop(equipment[slot]);
		int otherslot = item::def[itemobj->desc->data].otherslot;
		if (otherslot > 0 && equipment[otherslot])
			unequip_drop(equipment[otherslot]);
		equipment[slot] = itemobj->desc;
		itemobj->desc->room = room::equipment;
		if (otherslot > 0) equipment[otherslot] = itemobj->desc;
		recalc_stats();
	}

	 // Character stats (affected by items and such)
	MoveStats stats;
	void recalc_stats () {
		stats = default_stats;
		for (uint i=0; i < item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->slot == (int)i)  // Don't count otherslot
		if (equip_info(i)->stat_mod)
			(*equip_info(i)->stat_mod)(&stats);
	}

	 // controls
	void read_controls () {
		control_left = key[sf::Key::A];
		control_right = key[sf::Key::D];
		control_jump = key[sf::Key::W] && ((!floor) || key[sf::Key::W] < 3);
		control_kneel = key[sf::Key::S];
		control_action = key[sf::Key::Space] == 1;
		control_aim = button[sf::Mouse::Right] || key[sf::Key::LShift];
		control_click = control_aim ? button[sf::Mouse::Left] : button[sf::Mouse::Left] == 1;
		control_goto = button[sf::Mouse::Middle] == 1;
		if (control_left || control_right || control_jump || control_kneel
		 || control_action || control_aim) auto_control = false;
		else if (control_goto) {
			printf("Calculating route...\n");
			destination = map::get_platform(aim_center() + Vec(cursor.x, cursor.y));
			auto_control = true;
		}
		if (auto_control) {
			map::Pos plat = map::get_platform(pos());
			if (map::same_platform(plat, destination)) {
				if (destination.x < plat.x)
					control_left = true;
				else if (plat.x < destination.x)
					control_right = true;
			}
		}
		 // Dump debug info
		if (key[sf::Key::BackSlash] == 1) {
			map::debug_print();
			for (Actor* a = actors_by_depth; a; a = a->next_depth)
				a->debug_print();
		}
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
		aiming = control_aim;
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
		if (control_left && !control_right) {
			if (xvel() < 0)
				floor_friction = stats.run_accel;
			else floor_friction = stats.decel;
			if (facing < 0)
				ideal_xvel = !aiming ? -stats.run_speed : -stats.walk_speed;
			else
				ideal_xvel = -stats.walk_speed;
		}
		 // Right
		else if (control_right && !control_left) {
			if (xvel() > 0)
				floor_friction = stats.run_accel;
			else floor_friction = stats.decel;
			if (facing > 0)
				ideal_xvel = !aiming ? stats.run_speed : stats.walk_speed;
			else
				ideal_xvel = stats.walk_speed;
		}
		 // Stop
		else {
			floor_friction = stats.decel;
			ideal_xvel = 0;
		}
		return abs_f(xvel()) >= 0.01;
	}

	bool allow_jump () {
		if (control_jump) {  // Jump
			set_vel(Vec(vel().x, stats.jump_vel));
			float_frames = stats.float_time*FPS;
			state = falling;
			return true;
		}
		else return false;
	}

	bool allow_kneel () {
		return (control_kneel && floor_normal.y > 0.9)
		    || (state == crawling && check_fix(fix_sensor_21));
	}

	bool allow_crawl () {
		floor_friction = stats.decel;
		if (control_left && !control_right) {
			if (!crawling) facing = -1;
			ideal_xvel = -stats.crawl_speed;
			return true;
		}
		else if (control_right && !control_left) {
			if (!crawling) facing = 1;
			ideal_xvel = stats.crawl_speed;
			return true;
		}
		else {
			ideal_xvel = 0;
			return false;
		};
	}
	
	void allow_airmove () {
		 // Left
		if (control_left && !control_right) {
			float max = -stats.air_speed;
			if (xvel() > max) {
				add_vel(Vec(-stats.air_accel, 0));
				if (xvel() < max) {
					add_vel(Vec(-xvel() + max, 0));
				}
			}
		}
		 // Right
		else if (control_right && !control_left) {
			float max = stats.air_speed;
			if (xvel() < max) {
				add_vel(Vec(stats.air_accel, 0));
				if (xvel() > max) {
					add_vel(Vec(-xvel() + max, 0));
				}
			}
		}
		 // Adjust falling speed
		if (control_jump && float_frames) {
				body->SetGravityScale((stats.float_time-float_frames/FPS)/stats.float_time);
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
		 && control_click) {
			if (equip_info(item::hand))
			if (equip_info(item::hand)->use) {
				(*equip_info(item::hand)->use)();
			}
		}
	}

	void allow_action () {
		if (control_action)
		switch (action) {
			case action_equip: {
				pick_up_equip((Item*)action_arg);
				break;
			}
			case action_enter: {
				obj::Desc* d = ((Object*)action_arg)->desc;
				break;
			}
			default: { }
		}
	}

	void allow_examine () {
		if (!aiming) {
			if (abs_f(cursor.x) < 0.1 && abs_f(cursor.y) < 0.1) {
				can_see = true;
			}
			else if (cursor.x*facing < 0) {
				can_see = false;
			}
			else {
				b2Fixture* seeing = check_line(
					aim_center(), cursor_pos(), cf::sight_barrier.maskBits
				).hit;
				can_see = (seeing == NULL);
			}
			if (control_click) {
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
				else if (can_see && pointed_object && pointed_object->desc->id != obj::tilemap) {
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
		read_controls();
		fix_old = fix_current;
		fix_helmet_old = fix_helmet_current;
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
						if (facing > 0) {
							set_fix(fix_crawl_r);
							set_helmet(fix_helmet_crawl_r);
						}
						else {
							set_fix(fix_crawl_l);
							set_helmet(fix_helmet_crawl_l);
						}
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
						floor_friction = stats.decel;
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
					if (check_sensor_block() && !check_sensor_wall())
						min_aim = 0;
					else
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
					snd::def[snd::step].play(1.2, 8*-oldyvel);
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
						snd::def[snd::step].play(1.2, 8*-oldyvel/2.0);
						goto got_floor;
					}
					else {
						snd::def[snd::fall].play(0.9, 6*-oldyvel/2.0);
						goto hurt_floor;
					}
				}
				hurt_no_floor:
				if (hurt_frames == 0) goto no_floor;
				state = hurt_air;
				allow_look();
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
				floor_friction = stats.decel;
				ideal_xvel = 0;
				allow_look();
				decrement_counters();
				set_fix(fix_21);
				set_helmet(fix_helmet_kneel);
				break;
			}
			case dead_air: {
				if (floor && floor_normal.y > 0.9) {
					snd::def[snd::fall].play(0.9, 6*-oldyvel/2.0);
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
				floor_friction = stats.decel;
				ideal_xvel = 0;
				set_fix(fix_h7);
				set_helmet(-1);
				break;
			}	
		}

		if (floor) oldxrel = x() - floor->x();
		oldyvel = yvel();

		 // Select cursor image
		if (aiming) cursor.img = img::target;
		else if (message) cursor.img = img::readmore;
		else if (can_see) {
			if (pointed_object && pointed_object->desc->id != obj::tilemap)
				cursor.img = img::see;
			else cursor.img = img::look;
		}
		 // Test can_reach_with_jump
		//if (floor) {
		//	if (map::can_reach_with_jump(pos(), Vec(6.0, jump_velocity()), cursor_pos(), jump_float_time())) {
		//		cursor.img = img::see;
		//	}
		//	else cursor.img = img::nolook;
		//}
		else cursor.img = img::nolook;
		action = -1;
		action_distance = 1000000;
		
		update_fixtures();
		Walking::before_move();
	}

	virtual void damage (int d) {
		Object::damage(d);
		state = ouch;
		hurt_frames = (20 + d) / (2 + adrenaline/60.0);
		inv_frames = 50;
		adrenaline += 5*d;
		//printf("Hurt status: %d, %d, %d\n", hurt_frames, inv_frames, adrenaline);
		snd::def[snd::hurt].play(1.0 - 0.1*d/24, 40 + d);
	}

	virtual void kill () {
		trap_cursor = false;
		draw_cursor = false;
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
						snd::def[snd::step].play(0.9+rand()*0.2/RAND_MAX, 6*abs_f(xvel())*(1.0+rand()*0.2/RAND_MAX));
				}
			}
		}
		else distance_walked = 0;
	};
	Rata () : Walking() { rata = this; }
	virtual void on_create () {
		make_body(desc, true, true);
			for (uint i = obj::def[desc->id].nfixes; i > 0; i--) {
				dbg(4, "Fix %d: 0x%08x\n", i, body->CreateFixture(&(obj::def[desc->id].fixdef[i-1])));
			}
		dbg(3, "Affixed 0x%08x with 0x%08x\n", this, body);
		fix_old = fix_current = fix_27;
		fix_helmet_old = fix_helmet_current = -1;
		update_fixtures();
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
		for (uint i=0; i<MAX_INVENTORY; i++)
			inventory[i] = NULL;
		for (uint i=0; i<item::num_slots; i++)
			equipment[i] = NULL;
		for (uint i=0; i<n_saved_things; i++) {
			if (saved_things[i].room == room::equipment) {
				equipment[item::def[saved_things[i].data].slot] = &saved_things[i];
				if (item::def[saved_things[i].data].otherslot > -1)
					equipment[item::def[saved_things[i].data].otherslot] = &saved_things[i];
			}
			else if (saved_things[i].room <= -100) {
				inventory[saved_things[i].room + 100] = &saved_things[i];
			}
		}
		facing = desc->facing ? desc->facing : 1;
		cursor.x = 2.0 * facing;
		cursor.y = 0;
		cursor.img = img::look;
		trap_cursor = true;
		draw_cursor = true;
		oldyvel = 0.0;
		recalc_stats();
	}
	virtual void on_destroy () {
		rata = NULL;
	}

	uint pose_arm_by_aim () {
		using namespace pose;
		return
		     (state == standing || state == walking)
		  && check_sensor_block()
		  && !check_sensor_wall()
		                     ? Arm::angle_far[angle_frame]
		: recoil_frames > 20 ? Arm::angle_near[angle_frame] + 1
		: aim_distance > 10  ? Arm::angle_far[angle_frame]
		: aim_distance > 4   ? Arm::angle_mid[angle_frame]
		:                     Arm::angle_near[angle_frame];
	}

	void draw () {

		float step = state == crawling ? 0.8 : 1.0;
		int walk_frame;
		bool flip = facing<0;
		bool armflip = flip;
		bool forearmflip = flip;
		Vec p = pos();
		if (equip_info(item::feet))
		if (state == standing || state == walking) p.y += 1*PX;
		
		//if (inv_frames % 3 == 2) return;

		 // Select aim angle frame
		angle_frame = get_angle_frame(aim_direction);

		 // Select walking frame
		if (floor) {
			float step_d = mod_f(distance_walked, step * 2);
			if (step_d < 0) step_d += step*2;
			walk_frame =
			  abs_f(xvel()) < 0.01 ? 0
			: step_d < step*5/9.0  ? 1
			: step_d < step        ? 2
			: step_d < step*14/9.0 ? 3
			:                        0;
		}
		else walk_frame = 3;

		using namespace pose;

		switch (state) {
			case standing: {
				pose.body = Body::stand;
				pose.head = Head::angle_stand[angle_frame];
				if (aiming) {
					pose.arm = pose_arm_by_aim();
					pose.forearm = angle_frame + (recoil_frames > 20);
					pose.hand = angle_frame + (recoil_frames > 20);
				}
				else {
					pose.arm = a0;
					pose.forearm = a0;
					pose.hand = a0;
				}
				sight_points = sight_points_stand;
				break;
			}
			case walking: {
				pose.body = (walk_frame % 2) ? Body::walk
				                            : Body::stand;
				pose.head = (walk_frame % 2) ? Head::angle_walk[angle_frame]
				                            : Head::angle_stand[angle_frame];
				if (aiming) {
					pose.arm = pose_arm_by_aim();
					pose.forearm = angle_frame + (recoil_frames > 20);
					pose.hand = angle_frame + (recoil_frames > 20);
				}
				else {
					pose.arm = (int[4]){ a0, -a23, a0, a23 }[walk_frame];
					pose.forearm = pose.arm;
					pose.hand = (int[4]){ a0, a338, a0, a23 }[walk_frame];
				}
				sight_points = sight_points_stand;
				break;
			}
			case kneeling: {
				pose.body = Body::kneel;
				pose.head = Head::angle_stand[angle_frame];
				if (aiming) {
					pose.arm = pose_arm_by_aim();
					pose.forearm = angle_frame + (recoil_frames > 20);
					pose.hand = angle_frame + (recoil_frames > 20);
				}
				else {
					pose.arm = a45;
					pose.forearm = a68;
					pose.hand = a45;
				}
				sight_points = sight_points_kneel;
				break;
			}
			case crawling: {
				pose.body = (walk_frame % 2) ? Body::crawl2
				                            : Body::crawl;
				pose.head = Head::crawl;
				if (aiming) {
					pose.arm = a23;
					pose.forearm = angle_frame + (recoil_frames > 20);
					pose.hand = angle_frame + (recoil_frames > 20);
				}
				else {
					pose.arm = (int[4]){ a0, a45, a0, -a45 }[walk_frame];
					pose.forearm = a90;
					pose.hand = Hand::inside;
				}
				if (facing>0) sight_points = sight_points_crawl_r;
				else sight_points = sight_points_crawl_l;
				break;
			}
			case falling: {
				pose.body = Body::walk;
				pose.head = (yvel() < 0) ? Head::angle_walk[angle_frame]
				                        : Head::angle_stand[angle_frame];
				if (aiming) {
					pose.arm = pose_arm_by_aim();
					pose.forearm = angle_frame + (recoil_frames > 20);
					pose.hand = angle_frame + (recoil_frames > 20);
				}
				else {
					pose.arm = -a23;
					pose.forearm = a45;
					pose.hand = a45;
				}
				sight_points = sight_points_stand;
				break;
			}
			case ouch:
			case hurt_air:
			case dead_air: {
				pose.body = Body::hurtbk;
				pose.head = Head::hurtbk;
				pose.arm = a23;
				pose.forearm = a68;
				pose.hand = a68;
				sight_points = sight_points_kneel;
				break;
			}
			case hurt: {
				if (hurt_direction == 1) {
					pose.body = Body::crawl;
					pose.head = Head::crawl;
					pose.arm = a0;
					pose.forearm = a90;
					pose.hand = Hand::inside;
				}
				else {
					pose.body = Body::sit;
					pose.head = Head::hurtbk;
					pose.arm = -a23;
					pose.forearm = -a23;
					pose.hand = Hand::front;
				}
				sight_points = sight_points_kneel;
				break;
			}
			case dead: {
				pose.body = Body::laybk;
				pose.head = Head::laybk;
				pose.arm = a90;
				pose.forearm = a90;
				pose.hand = Hand::inside;
				sight_points = sight_points_crawl_r;
				break;
			}
			default: {
				printf("Warning: Bad state %d encountered.\n", state);
				break;
			}
		}
		helmet_angle = head[pose.head].helmeta;
		if (    pose.arm < a0  )     pose.arm =         -    pose.arm,     armflip = !    armflip;
		if (    pose.arm > a180)     pose.arm = (2*a180)-    pose.arm,     armflip = !    armflip;
		if (pose.forearm < a0  ) pose.forearm =         -pose.forearm, forearmflip = !forearmflip;
		if (pose.forearm > a180) pose.forearm = (2*a180)-pose.forearm, forearmflip = !forearmflip;
		
		 // Get offsets
		Vec headp =        pose::body[   pose.body].head   .scalex(facing);
		Vec helmetp =    headp + head[   pose.head].helmet .scalex(facing);
		Vec armp =         pose::body[   pose.body].arm    .scalex(facing);
		Vec forearmp =     armp + arm[    pose.arm].forearm.scalex(armflip?-1:1);
		hand_pos = forearmp + forearm[pose.forearm].hand   .scalex(forearmflip?-1:1);

		 // Now to actually draw.

		draw_image(img::rata_body, p, pose.body, flip);
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->body > -1)
			draw_image(equip_info(i)->body, p, pose.body, flip);

		if (state == dead) goto draw_arm;
		draw_head:
		draw_image(
			img::rata_head,
			p + headp,
			pose.head, flip
		);
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->head > -1)
			draw_image(
				equip_info(i)->head,
				p + headp,
				pose.head, flip
			);
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->helmet > -1)
			draw_image(
				equip_info(i)->helmet,
				p + helmetp,
				pose::head[pose.head].helmetf, flip
			);

		if (state == dead) goto draw_hand;
		draw_arm:
		draw_image(
			img::rata_arm,
			p + armp,
			pose.arm, armflip
		);
		draw_image(
			img::rata_forearm,
			p + forearmp,
			pose.forearm, forearmflip
		);
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->arm > -1)
			draw_image(
				equip_info(i)->arm,
				p + armp,
				pose.arm, armflip
			);
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->forearm > -1)
			draw_image(
				equip_info(i)->forearm,
				p + forearmp,
				pose.forearm, forearmflip
			);

		if (state == dead) goto draw_head;
		
		draw_hand:
		for (uint i=0; i<item::num_slots; i++)
		if (equip_info(i))
		if (equip_info(i)->hand > -1)
			draw_image(
				equip_info(i)->hand,
				p + hand_pos,
				pose.hand, flip
			);
		
		 // Draw action message
		const char* m =
		  action == action_equip ? "EQUIP"
		: action == action_enter ? "ENTER"
		:                          NULL;
		if (m) {
			render_text((char*)m, pos() + Vec(0, 3), 1, true, true, 0);
		}

	}
};



