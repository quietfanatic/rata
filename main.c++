#include "top.c++"


int main () {
	load_img();
	load_snd();
	main_init();

	rata = (Rata*)obj::Desc(
		room::everywhere, obj::rata,
		Vec(10, 10)
	).manifest();
	obj::Desc(
		room::everywhere, obj::lifebar
	).manifest();
	obj::Desc(
		room::everywhere, obj::shade,
		Vec(0, 0), Vec(20, 15),
		-1, 0x0f0f0f4f
	).manifest();

	load_save();

	enter_room(room::roompicker, 0);
	try { main_loop(); } catch (int x) { printf("Game quit succesfully with result %d.\n", x); }
	return 0;
}









