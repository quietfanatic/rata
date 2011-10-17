#include "top.c++"


int main () {
	load_img();
	main_init();

	/*
	obj::Desc(obj::tilemap, &tiles::testroom).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(20, 1), cf::solid, 0.4)}, 10, 0.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(1, 15), cf::solid, 0.4)}, 0.5, 7.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(1, 15), cf::solid, 0.4)}, 19.5, 7.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(20, 1), cf::solid, 0.4)}, 10, 14.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(4, 1), cf::solid, 0.4)}, 3, 6.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(5, 1), cf::solid, 0.4)}, 8.5, 9.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(4, 1), cf::solid, 0.4)}, 8, 3.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(1, 4), cf::solid, 0.4)}, 10.5, 12).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(1, 5), cf::solid, 0.4)}, 13.5, 3.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(2, 1), cf::solid, 0.4)}, 18, 4.5).manifest();
	obj::Desc(obj::rata, NULL, 1.5, 3).manifest();
	obj::Desc(obj::rat, NULL, 16, 10).manifest();
	obj::Desc(obj::rat, NULL, 12, 10).manifest();
	obj::Desc(obj::rat, NULL, 7.5, 12).manifest();
	obj::Desc(obj::crate, NULL, 8.5, 10.5).manifest();
	*/
//	FILE* F = fopen("rooms/outputtest.room.c++", "w");
//	if (F == NULL)
//		printf("Failed to open F\n");
//	if (room::file::test1::room.print_to_file(F)) {
//		printf("Room write succeeded.\n");
//	}
//	else {
//		printf("Room write failed.\n");
//	}
	rata = (Rata*)obj::Desc(
		obj::rata, NULL,
		10, 10
	).manifest();
	room::file::test2::room.enter(0);

	try { main_loop(); } catch (int x) { printf("Game quit succesfully with result %d.\n", x); }
	return 0;
}









