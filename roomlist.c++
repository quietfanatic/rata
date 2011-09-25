

namespace file {
	namespace edit1 {
		#include "rooms/edit1.room.c++"
	}
	namespace edit2 {
		#include "rooms/edit2.room.c++"
	}
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

struct room_pair {
	char* k;
	Room* v;
	Room* get (char* k_) {
		if (strcmp(k, k_) == 0) return v;
		else return NULL;
	}
} list [] = {
	{"edit1", &file::edit1::room},
	{"edit2", &file::edit2::room},
	{"test1", &file::test1::room},
	{"test2", &file::test2::room},
	{"test3", &file::test3::room},
	{NULL, NULL}
};
Room* name (char* k) {
	for (room_pair* p = list; p->k; p++) {
		if (Room* r = p->get(k)) return r;
	}
	return NULL;
}
#endif

