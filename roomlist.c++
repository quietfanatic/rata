
extern Def def [];
static Def*const roompicker = def+0;
static Def*const test1 = def+1;
static const uint n_rooms = 2;

namespace file {
	namespace roompicker {
		#define THIS_ROOM room::roompicker
		#include "rooms/0-roompicker.room.c++"
		#undef THIS_ROOM
	}
	namespace test1 {
		#define THIS_ROOM room::test1
		#include "rooms/1-test1.room.c++"
		#undef THIS_ROOM
	}
}

Def def [n_rooms] = {
	file::roompicker::def,
	file::test1::def
};


