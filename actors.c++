
#ifdef HEADER

struct Actor;

namespace actor {
	struct Def {
		int16 loc;
		int16 type;
		Vec pos;
		Vec vel;
		int32 facing;
		uint32 data;
		uint32 data2;

		Actor* manifest ();
		void debug_print ();

	};

	const uint n_globals = 7;
	extern Def saved [n_globals];
	extern Actor* global [n_globals];
}

#else


actor::Def actor::saved [actor::n_globals] = {

	{-1, type::game, Vec(0, 0), Vec(0, 0), 0, 0, 0},
	{2, type::rata, Vec(2, 2), Vec(0, 0), 0, 0, 0},
	{0, type::room, Vec(0, 0), Vec(0, 0), 0, room::roompicker, 0},
	{-1, type::back_tiles, Vec(0, 0), Vec(0, 0), 0, 0, 0},
	{-1, type::bullet_layer, Vec(0, 0), Vec(0, 0), 0, 0, 0},
	{-1, type::front_tiles, Vec(0, 0), Vec(0, 0), 1, 0, 0},
	{-1, type::shade, Vec(0, 0), Vec(0, 0), 0, 0x0000004f, 0}



};


Actor* actor::global [actor::n_globals];


void init_objects () {
	for (uint i=0; i < actor::n_globals; i++) {
		actor::global[i] = actor::saved[i].manifest();
	}
	for (uint i=0; i < actor::n_globals; i++) {
		if (actor::saved[i].loc > -1)
			actor::global[actor::saved[i].loc]->take(actor::global[i]);
	}
	for (uint i=0; i < actor::n_globals; i++) {
		if (actor::saved[i].loc < 0)
			actor::global[i]->activate();
	}
	actor::global[actor::global[1]->loc]->activate();  // Rata's room
}

namespace actor {
	Actor* Def::manifest () {
		return type::def[type].alloc(this);
	}
	void Def::debug_print () {
		printf("%08x %12s: (% 8.4f, % 8.4f) @ (% 8.4f, % 8.4f) % d; %d %d\n",
			this, type::def[type].name,
			pos.x, pos.y,
			vel.x, vel.y,
			facing, data, data2
		);
	}
}






#endif

