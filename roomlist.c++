

enum ID {
	roompicker,
	test1,
	n_rooms
};

namespace file {
	namespace roompicker { extern Room room; }
	namespace test1 { extern Room room; }
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

Room* list [] = {
	&file::roompicker::room,
	&file::test1::room
};

#ifdef MAPEDITOR
char* filename [] = {
	"rooms/0-roompicker.room.c++",
	"rooms/1-test1.room.c++"
};
#endif

