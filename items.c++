
#ifdef HEADER

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
		int8 slot;
		int8 otherslot;
		
		int16 appearance;
		uint16 world_frame;
		uint16 inventory_frame;
		int16 body;
		int16 head;
		int16 helmet;
		int16 arm;
		int16 forearm;
		int16 hand;
		void (* use ) ();
		void (* stat_mod ) (MoveStats* s);
	};


	void fire_handgun ();
	void decrease_float_time (MoveStats* s);


	Def def [] = {
		{	"White dress",
			body, -1,
			img::dress_body, pose::Body::laybk, pose::Body::stand,
			img::dress_body, -1, -1, img::dress_arm, img::dress_forearm, -1,
			NULL, NULL
		},
		{	"Handgun",	
			hand, -1,
			img::handgun, 19, pose::a45,
			-1, -1, -1, -1, -1, img::handgun,
			&fire_handgun, NULL
		},
		{	"Boots",
			feet, -1,
			img::boots, pose::Body::stand, pose::Body::stand,
			img::boots, -1, -1, -1, -1, -1,
			NULL, NULL
		},
		{	"Helmet",
			head, -1,
			img::helmet, 0, 2,
			-1, -1, img::helmet, -1, -1, -1,
			NULL, &decrease_float_time
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
	snd::def[snd::gunshot].play(1.0, 80);
}

void item::decrease_float_time (MoveStats* s) {
	s->float_time *= 0.8;
}


#endif




