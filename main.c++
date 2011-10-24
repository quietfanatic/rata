#include "top.c++"


int main () {
	load_img();
	load_snd();
	main_init();

	rata = (Rata*)obj::Desc(
		-1, obj::rata,
		Vec(10, 10)
	).manifest();

	load_save();

	enter_room(room::roompicker, 0);
	bgm::music.OpenFromFile(bgm::name[bgm::life]);
	bgm::music.SetVolume(80);
	bgm::music.Play();
	try { main_loop(); } catch (int x) { printf("Game quit succesfully with result %d.\n", x); }
	return 0;
}









