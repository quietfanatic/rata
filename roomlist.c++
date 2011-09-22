

namespace file {
	namespace test1 {
		#include "rooms/test1.room.c++"
	}
	namespace test2 {
		#include "rooms/test2.room.c++"
	}
	namespace test3 {
		#include "rooms/test3.room.c++"
	}
};
#ifdef MAPEDITOR

std::unordered_map<char*, Room*> name;

void load_rooms () {
	name["test1"] = &file::test1::room;
	name["test2"] = &file::test2::room;
	name["test3"] = &file::test3::room;
};
#endif

