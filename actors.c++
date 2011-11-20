
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

	};

	const uint n_globals = 40;  // Make sure this isn't too big!
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
	{ -1, type::room     , Vec(   0,   0), Vec(  0,  0),  0, room::roompicker, 0},
	{ -1, type::room     , Vec(  20,   0), Vec(  0,  0),  0, room::test1, 0},
	{ -1, type::room     , Vec(   0,  15), Vec(  0,  0),  0, room::test2, 0},
	{ -1, type::room     , Vec(  40, -10), Vec(  0,  0),  0, room::dangent, 0},
#define A 4
	{R+0, type::rata     , Vec(   2,   2), Vec(  0,  0),  0, 0, 0},
	{A+0, type::item     , Vec(  -1,   0), Vec(  0,  0),  0, item::white_dress, 0},
	{R+0, type::bg_color , Vec(   0,   0), Vec( 20, 15),  0, 0x7f7f7fff, 0},
	{R+0, type::item     , Vec(   8,   5), Vec(  0,  0),  0, item::handgun, 0},
	{R+0, type::door     , Vec( 8.5,   1), Vec( -7, 35),  0, 0, 0},
	{R+2, type::door     , Vec( 1.5,  36), Vec(  7,-35),  0, 0, 0},
	{R+1, type::bg_color , Vec(  20,   0), Vec( 20, 15),  0, 0x7f7f7fff, 0},
	{R+1, type::rat      , Vec(  29,  10), Vec(  0,  0),  1, 0, 0},
	{R+1, type::rat      , Vec(  32,   1), Vec(  0,  0),  1, 0, 0},
	{R+2, type::bg_color , Vec(   0,  15), Vec( 40, 30),  0, 0x7f7f7fff, 0},

	{R+2, type::rat      , Vec(  13,  34), Vec(  0,  0),  1, 0, 0},
	{R+2, type::rat      , Vec(  17,  34), Vec(  0,  0),  1, 0, 0},
	{R+2, type::rat      , Vec(  29,  34), Vec(  0,  0),  1, 0, 0},
	{R+2, type::rat      , Vec(  32,  34), Vec(  0,  0),  1, 0, 0},
	{R+2, type::patroller, Vec(  14,  23), Vec(  0,  0), -1, 0, 0},
	{R+2, type::patroller, Vec(  32,  27), Vec(  0,  0), -1, 0, 0},
	{R+2, type::crate    , Vec(10.5,  32), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec( 6.5,  20), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec( 7.5,  20), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec( 7.0,  21), Vec(  0,  0),  1, 0, 0},

	{R+2, type::crate    , Vec(21.5,  30), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec(22.5,  30), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec(22.5,  31), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec(20.5,  18), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec(21.5,  19), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec(21.5,  18), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec(20.5,  19), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec( 1.5,  16), Vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , Vec( 2.5,  16), Vec(  0,  0),  1, 0, 0},
	{R+3, type::bg_color , Vec(  40, -10), Vec( 30, 20),  0, 0x7f7f7fff, 0},

	{ -2, type::back_tiles, Vec(0, 0), Vec(0, 0), 0, 0, 0},
	{ -2, type::bullet_layer, Vec(0, 0), Vec(0, 0), 0, 0, 0},
	{ -2, type::front_tiles, Vec(0, 0), Vec(0, 0), 1, 0, 0},
	{ -2, type::cursor_layer, Vec(0, 0), Vec(0, 0), 1, 0, 0},
	{ -2, type::shade, Vec(0, 0), Vec(20, 15), -1, 0x0000004f, 0},
	{ -2, type::lifebar, Vec(0, 0), Vec(0, 0), 0, 0, 0},

};

Actor* actor::global [actor::n_globals];

Room** room::global = (Room**)actor::global;

void init_objects () {
	 // Create actors
	printf("CREATING\n");
	for (uint i=0; i < actor::n_globals; i++) {
		actor::global[i] = actor::saved[i].manifest();
	}
	for (uint i=0; i < actor::n_globals; i++) {
		actor::global[i]->debug_print();
	}
	 // Locate actors
	printf("LOCATING\n");
	for (uint i=0; i < actor::n_globals; i++) {
		if (actor::global[i]->loc > -1)
			actor::global[actor::global[i]->loc]->receive(actor::global[i]);
	}
	for (uint i=0; i < actor::n_globals; i++) {
		actor::global[i]->debug_print();
	}
	 // Activate 'everywhere' actors
	printf("ACTIVATING\n");
	for (uint i=0; i < actor::n_globals; i++) {
		if (actor::global[i]->loc == -2)
			actor::global[i]->activate();
	}
	for (uint i=0; i < actor::n_globals; i++) {
		actor::global[i]->debug_print();
	}
	 // Enter Rata's room.
	printf("ENTERING\n");
	if (actor::global[actor::global[A+0]->loc]->type != type::room) {
		printf("Warning: Rata did not start out in a room.\n\tEjecting to room 0.\n");
		actor::global[A+0]->loc = 0;
		actor::global[A+0]->pos = actor::global[0]->pos + Vec(1, 1);
		room::global[0]->enter();
	}
	else {
		room::global[actor::global[A+0]->loc]->enter();
	}
	for (uint i=0; i < actor::n_globals; i++) {
		actor::global[i]->debug_print();
	}
}

namespace actor {
	Actor* Def::manifest () {
		return type::def[type].alloc(this);
	}
}


#undef R
#undef A


#endif

