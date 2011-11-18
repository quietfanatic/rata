

enum ID {
	roompicker,
	n_rooms
};

namespace file {
	namespace roompicker { extern Room room; }
	namespace roompicker {
		#define THIS_ROOM room::roompicker
		#include "rooms/0-roompicker.room.c++"
	}
}

Room* list [] = {
	&file::roompicker::room
};

#ifdef MAPEDITOR
char* filename [] = {
	"rooms/0-roompicker.room.c++"
};
#endif

