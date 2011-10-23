#include "top.c++"


int main () {
	img::load_img();
	load_snd();
	main_init();

	rata = (Rata*)obj::Desc(
		-1, obj::rata,
		Vec(10, 10)
	).manifest();

	room::file::roompicker::room.enter(0);
	bgm::life.SetVolume(80);
	bgm::life.Play();
	try { main_loop(); } catch (int x) { printf("Game quit succesfully with result %d.\n", x); }
	return 0;
}









