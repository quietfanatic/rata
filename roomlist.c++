

enum ID {
	roompicker,
	test1,
	n_rooms
};

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

Desc desc [n_rooms] = {
	file::roompicker::desc,
	file::test1::desc
};


