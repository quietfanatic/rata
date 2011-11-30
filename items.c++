
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
	struct Def {
		char* name;
		int8 slot;
		int8 otherslot;
		
		img::Def* appearance;
		uint16 world_frame;
		uint16 inventory_frame;
		img::Def* body;
		img::Def* head;
		img::Def* helmet;
		img::Def* arm;
		img::Def* forearm;
		img::Def* hand;
		void (* use ) ();
		void (* stat_mod ) (MoveStats* s);
	};


	void fire_handgun ();
	void decrease_float_time (MoveStats* s);


	Def def [] = {
		{	"White dress",
			body, -1,
			img::dress_body, pose::Body::laybk, pose::Body::stand,
			img::dress_body, NULL, NULL, img::dress_arm, img::dress_forearm, NULL,
			NULL, NULL
		},
		{	"Handgun",	
			hand, -1,
			img::handgun, 19, pose::a45,
			NULL, NULL, NULL, NULL, NULL, img::handgun,
			&fire_handgun, NULL
		},
		{	"Boots",
			feet, -1,
			img::boots, pose::Body::stand, pose::Body::stand,
			img::boots, NULL, NULL, NULL, NULL, NULL,
			NULL, NULL
		},
		{	"Helmet",
			head, -1,
			img::helmet, 0, 2,
			NULL, NULL, img::helmet, NULL, NULL, NULL,
			NULL, &decrease_float_time
		},
	};
	Def*const white_dress = def+0;
	Def*const handgun = def+1;
	Def*const boots = def+2;
	Def*const helmet = def+3;
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
		rata->pos + rata->hand_pos
		          + vec(handgun_bulletx[rata->angle_frame]*rata->facing,
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




