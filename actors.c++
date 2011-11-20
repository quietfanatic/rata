
#ifdef HEADER

struct Actor;
struct Room;

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

	const uint n_globals = 13;  // Make sure this isn't too big!
	extern Def saved [n_globals];
	extern Actor* global [n_globals];
}
namespace room {
	extern Room** global;
}

#else


actor::Def actor::saved [actor::n_globals] = {
 // Rooms are first
#define R 0
	{ -1, type::room     , Vec(  0,  0), Vec(  0,  0),  0, room::roompicker, 0},
	{ -1, type::room     , Vec( 20,  0), Vec(  0,  0),  0, room::test1, 0},
	{ -1, type::room     , Vec(  0, 15), Vec(  0,  0),  0, room::test2, 0},
#define A 3
	{R+0, type::rata     , Vec(  2,  2), Vec(  0,  0),  0, 0, 0},
	{A+0, type::item     , Vec( -1,  0), Vec(  0,  0),  0, item::white_dress, 0},
	{R+0, type::item     , Vec(  8,  5), Vec(  0,  0),  0, item::handgun, 0},
	{R+1, type::rat      , Vec( 29, 10), Vec(  0,  0),  0, 0, 0},
	{R+1, type::rat      , Vec( 32,  1), Vec(  0,  0),  0, 0, 0},

	{ -2, type::back_tiles, Vec(0, 0), Vec(0, 0), 0, 0, 0},
	{ -2, type::bullet_layer, Vec(0, 0), Vec(0, 0), 0, 0, 0},
	{ -2, type::front_tiles, Vec(0, 0), Vec(0, 0), 1, 0, 0},
	{ -2, type::shade, Vec(0, 0), Vec(0, 0), 0, 0x0000004f, 0},
	{ -2, type::lifebar, Vec(0, 0), Vec(0, 0), 0, 0, 0},

};

Actor* actor::global [actor::n_globals];
Room** room::global = (Room**)actor::global;

void init_objects () {
	 // Create actors
	for (uint i=0; i < actor::n_globals; i++) {
		actor::global[i] = actor::saved[i].manifest();
	}
	 // Locate actors
	for (uint i=0; i < actor::n_globals; i++) {
		if (actor::saved[i].loc > -1)
			actor::global[actor::saved[i].loc]->receive(actor::global[i]);
	}
	 // Activate 'everywhere' actors
	for (uint i=0; i < actor::n_globals; i++) {
		if (actor::saved[i].loc == -2)
			actor::global[i]->activate();
	}
	 // Enter Rata's room.
	if (actor::global[actor::global[A+0]->loc]->type != type::room) {
		printf("Warning: Rata did not start out in a room.\n\tEjecting to room 0.\n");
		actor::global[A+0]->loc = 0;
		actor::global[A+0]->pos = actor::global[0]->pos + Vec(1, 1);
		room::global[0]->enter();
	}
	else {
		room::global[actor::global[A+0]->loc]->enter();
	}
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


#undef R
#undef A


#endif

