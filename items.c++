
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
	enum ID {
		white_dress,
		handgun,
		boots,
		helmet,
	};

	struct Def {
		char* name;
		uint slot;
		uint otherslot;
		
		img::Image* appearance;
		uint world_frame;
		uint inventory_frame;
		img::Image* body;
		img::Image* head;
		img::Image* helmet;
		img::Image* arm;
		img::Image* forearm;
		img::Image* hand;
		void (* use ) ();
	};


	void fire_handgun ();


	Def def [] = {
		{	"White dress",
			body, -1,
			&img::dress_body, pose::body::laybk, pose::body::stand,
			&img::dress_body, NULL, NULL, &img::dress_arm, &img::dress_forearm, NULL, NULL
		},
		{	"Handgun",	
			hand, -1,
			&img::handgun, 19, pose::hand::a45,
			NULL, NULL, NULL, NULL, NULL, &img::handgun, &fire_handgun
		},
		{	"Boots",
			feet, -1,
			&img::boots, pose::body::stand, pose::body::stand,
			&img::boots, NULL, NULL, NULL, NULL, NULL, NULL
		},
		{	"Helmet",
			head, -1,
			&img::helmet, 0, 2,
			NULL, NULL, &img::helmet, NULL, NULL, NULL, NULL
		},
	};
}

#else

float handgun_bulletx [] = {
	2*PX, 3*PX, 5*PX, 5*PX, 5*PX, 4*PX, 3*PX, 0*PX, -2*PX
};
float handgun_bullety [] = {
	-5*PX, -4*PX, -3*PX, 0*PX, 2*PX, 3*PX, 5*PX, 5*PX, 5*PX
};
void item::fire_handgun () {
	rata->recoil_frames = 30;
	//rata->fire_bullet(
	//	rata->x() + rata->handx
	//	          + item::handgun_bulletx[rata->angle_frame]*rata->facing,
	//	rata->y() + rata->handy
	//	          + item::handgun_bullety[rata->angle_frame],
	//	rata->aim_direction
	//);
	fire_rbullet_dir(
		rata,
		rata->pos() + rata->hand_pos
		            + Vec(handgun_bulletx[rata->angle_frame]*rata->facing,
		                  handgun_bullety[rata->angle_frame]),
		rata->aim_direction,
		120
	);
	snd::gunshot.play(1.0, 80);
}

#endif




