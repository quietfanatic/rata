
#ifdef DEF_ONLY

namespace item {

	struct Equip {
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
	
		img::Image* body;
		img::Image* head;
		img::Image* arm;
		img::Image* hand;
		void (* use ) ();
	};


	Equip white_dress = {1,1,1,1,1,1,1,1,1, 4,
		&img::dress_body,
		NULL,
		NULL,
		NULL,
		NULL
	};

	void fire_handgun ();
	uint handgun_bulletx [] = {
		2*PX, 4*PX, 6*PX, 6*PX, 5*PX, 4*PX, 4*PX, 0*PX, -2*PX
	};
	uint handgun_bullety [] = {
		-5*PX, -4*PX, -4*PX, 0*PX, 2*PX, 4*PX, 6*PX, 6*PX, 5*PX
	};
	
	Equip handgun = {1,1,1,1,1,1,1,1,1, 0,
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
		  rata->x()
		+ pose::body::armx[rata->bodypose]*rata->facing
		+ pose::arm::handx[rata->armpose]*rata->facing
		+ item::handgun_bulletx[rata->handpose]*rata->facing,
		  rata->y()
		+ pose::body::army[rata->bodypose]
		+ pose::arm::handy[rata->armpose]
		+ item::handgun_bullety[rata->handpose],
		bullet_velocity * cos(rata->aim_direction),
		bullet_velocity * sin(rata->aim_direction),
		0,
		true
	))->manifest();
}

#endif




