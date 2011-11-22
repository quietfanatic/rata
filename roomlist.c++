
extern Def def [];
Def*const roompicker = def+0;
Def*const test1 = def+1;
Def*const test2 = def+2;
Def*const dangent = def+3;
const uint n_rooms = 4;

namespace file {
	namespace roompicker {
		#define HERE room::roompicker
		#include "rooms/0-roompicker.room.c++"
		#undef HERE
	}
	namespace test1 {
		#define HERE room::test1
		#include "rooms/1-test1.room.c++"
		#undef HERE
	}
	namespace test2 {
		#define HERE room::test2
		#include "rooms/2-test2.room.c++"
		#undef HERE
	}
	namespace dangent {
		#define HERE room::dangent
		#include "rooms/3-dangent.room.c++"
		#undef HERE
	}
}

Def def [n_rooms] = {
	file::roompicker::def,
	file::test1::def,
	file::test2::def,
	file::dangent::def
};


