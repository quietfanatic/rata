

enum ID {
	roompicker,
	n_rooms
};

namespace file {
	namespace roompicker {
		#define THIS_ROOM room::roompicker
		#include "rooms/0-roompicker.room.c++"
		#undef THIS_ROOM
	}
}

Def def [n_rooms] = {
	file::roompicker::def
};


