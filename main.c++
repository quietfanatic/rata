
#include "top.c++"

int main () {
	load_img();
	load_snd();
	main_init();
	
	rata = new Rata(type::rata, room::roompicker, Vec(2, 2));
	rata->pickup_equip(new Item(type::item, NULL, rata->pos, item::def+item::white_dress));
	(new TileLayer(type::back_tiles))->activate();
	(new BulletLayer(type::bullet_layer))->activate();
	(new TileLayer(type::front_tiles))->activate();
	(new CursorLayer(type::cursor_layer))->activate();
	(new Lifebar(type::lifebar))->activate();
	for (uint i=0; i < room::n_rooms; i++) {
		room::def[i].init();
	}

	rata->loc->enter();

	camera_jump = true;

	try { main_loop(); } catch (int x) { printf("Game quit succesfully with result %d.\n", x); }
	return 0;
}









