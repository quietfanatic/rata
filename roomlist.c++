

enum id {
	test1,
	test2,
	test3,
};

#include "rooms/test1.room.c++"
#include "rooms/test2.room.c++"
#include "rooms/test3.room.c++"


Room list [] = {
	r_test1,
	r_test2,
	r_test3,
};

