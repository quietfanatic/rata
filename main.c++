
#include "top.c++"

int main () {
	load_img();
	load_snd();
	main_init();
	
	rata = new Rata(room::roompicker, Vec(2, 2));
	rata->pickup_equip(new Item(NULL, rata->pos, item::white_dress));
	(new TileLayer(type::back_tiles))->activate();
	(new BulletLayer())->activate();
	(new TileLayer(type::front_tiles))->activate();
	(new CursorLayer())->activate();
	(new Lifebar())->activate();
	for (uint i=0; i < room::n_rooms; i++) {
		room::def[i].init();
	}

	rata->loc->enter();

	camera_jump = true;

	try { main_loop(); } catch (int x) { printf("Game quit succesfully with result %d.\n", x); }
	return 0;
}









