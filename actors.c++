
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

	const uint n_globals = 41;  // Make sure this isn't too big!
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
	{ -1, type::room     , vec(   0,   0), vec(  0,  0),  0, room::roompicker, 0},
	{ -1, type::room     , vec(  20,   0), vec(  0,  0),  0, room::test1, 0},
	{ -1, type::room     , vec(   0,  15), vec(  0,  0),  0, room::test2, 0},
	{ -1, type::room     , vec(  40, -10), vec(  0,  0),  0, room::dangent, 0},
#define A 4
	{R+0, type::rata     , vec(   2,   2), vec(  0,  0),  0, 0, 0},
	{A+0, type::item     , vec(  -1,   0), vec(  0,  0),  0, item::white_dress, 0},
	{R+0, type::bg_color , vec(   0,   0), vec( 20, 15),  0, 0x7f7f7fff, 0},
	{R+0, type::item     , vec(   8,   5), vec(  0,  0),  0, item::handgun, 0},
	{R+0, type::door     , vec( 8.5,   1), vec( -7, 35),  0, 0, 0},
	{R+2, type::door     , vec( 1.5,  36), vec(  7,-35),  0, 0, 0},
	{R+1, type::bg_color , vec(  20,   0), vec( 20, 15),  0, 0x7f7f7fff, 0},
	{R+1, type::rat      , vec(  29,  10), vec(  0,  0),  1, 0, 0},
	{R+1, type::rat      , vec(  32,   1), vec(  0,  0),  1, 0, 0},
	{R+2, type::bg_color , vec(   0,  15), vec( 40, 30),  0, 0x7f7f7fff, 0},

	{R+2, type::rat      , vec(  13,  34), vec(  0,  0),  1, 0, 0},
	{R+2, type::rat      , vec(  17,  34), vec(  0,  0),  1, 0, 0},
	{R+2, type::rat      , vec(  29,  34), vec(  0,  0),  1, 0, 0},
	{R+2, type::rat      , vec(  32,  34), vec(  0,  0),  1, 0, 0},
	{R+2, type::patroller, vec(  14,  23), vec(  0,  0), -1, 0, 0},
	{R+2, type::patroller, vec(  32,  27), vec(  0,  0), -1, 0, 0},
	{R+2, type::crate    , vec(10.5,  32), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec( 6.5,  20), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec( 7.5,  20), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec( 7.0,  21), vec(  0,  0),  1, 0, 0},

	{R+2, type::crate    , vec(21.5,  30), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec(22.5,  30), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec(22.5,  31), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec(20.5,  18), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec(21.5,  19), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec(21.5,  18), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec(20.5,  19), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec( 1.5,  16), vec(  0,  0),  1, 0, 0},
	{R+2, type::crate    , vec( 2.5,  16), vec(  0,  0),  1, 0, 0},
	{R+3, type::bg_color , vec(  40, -10), vec( 30, 20),  0, 0x7f7f7fff, 0},
	{R+3, type::flyer    , vec(57.5,-5.5), vec(  0,  0),  0, 0, 0},

	{ -2, type::back_tiles, vec(0, 0), vec(0, 0), 0, 0, 0},
	{ -2, type::bullet_layer, vec(0, 0), vec(0, 0), 0, 0, 0},
	{ -2, type::front_tiles, vec(0, 0), vec(0, 0), 1, 0, 0},
	{ -2, type::cursor_layer, vec(0, 0), vec(0, 0), 1, 0, 0},
	{ -2, type::shade, vec(0, 0), vec(20, 15), -1, 0x0000004f, 0},
	{ -2, type::lifebar, vec(0, 0), vec(0, 0), 0, 0, 0},

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
		actor::global[A+0]->pos = actor::global[0]->pos + vec(1, 1);
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

