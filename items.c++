
#ifdef DEF_ONLY

namespace item {
	enum Slot {
		feet,
		body,
		head,
		arm,
		hand,
		num_slots
	};

	struct Equip {
		uint slot;
		uint otherslot;
		float ground_accel;
		float ground_decel;
		float max_forward_speed;
		float max_backward_speed;
		float air_accel;
		float max_air_speed;
		float jump_velocity;
		float jump_float_time;
		float damage_mult;
		uint capacity;
		
		img::Image* appearance;
		uint world_frame;
		uint inventory_frame;
		img::Image* body;
		img::Image* head;
		img::Image* arm;
		img::Image* forearm;
		img::Image* hand;
		void (* use ) ();
	};


	Equip white_dress = {
		body, -1,
		1,1,1,1,1,1,1,1,1, 4,
		&img::dress_body, pose::body::laybk, pose::body::stand,
		&img::dress_body,
		NULL,
		&img::dress_arm,
		&img::dress_forearm,
		NULL,
		NULL
	};

	void fire_handgun ();
	float handgun_bulletx [] = {
		2*PX, 3*PX, 5*PX, 5*PX, 5*PX, 4*PX, 3*PX, 0*PX, -2*PX
	};
	float handgun_bullety [] = {
		-5*PX, -4*PX, -3*PX, 0*PX, 2*PX, 3*PX, 5*PX, 5*PX, 5*PX
	};
	
	Equip handgun = {
		hand, -1,
		1,1,1,1,1,1,1,1,1, 0,
		&img::handgun, 19, pose::hand::a45,
		NULL,
		NULL,
		NULL,
		NULL,
		&img::handgun,
		&fire_handgun
	};
}

#else

void item::fire_handgun () {
	rata->recoil_frames = 30;
	float bullet_velocity = 120.0;
	(new obj::Desc(obj::bullet, rata,
		rata->x() + rata->handx
		          + item::handgun_bulletx[rata->angle_frame]*rata->facing,
		rata->y() + rata->handy
		          + item::handgun_bullety[rata->angle_frame],
		bullet_velocity * cos(rata->aim_direction),
		bullet_velocity * sin(rata->aim_direction),
		0,
		true
	))->manifest();
	snd::gunshot.play(1.0, 80);
}

#endif




