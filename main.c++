#include "top.c++"


int main () {
	load_img();
	load_snd();
	main_init();

	load_save();

	enter_room(room::roompicker, 0);
	try { main_loop(); } catch (int x) { printf("Game quit succesfully with result %d.\n", x); }
	return 0;
}









