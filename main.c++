#include "top.c++"


int main () {
	load_img();
	load_snd();
	main_init();
	load_save();
	room::list[room::currenti]->load();

	try { main_loop(); } catch (int x) { printf("Game quit succesfully with result %d.\n", x); }
	return 0;
}









