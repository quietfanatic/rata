
#include "top.c++"

int main () {
	main_init();
	load_img();
	load_snd();
	 // Construct room camera constraint geometry
	for (uint i=0; i < room::n_rooms; i++)
	for (uint j=0; j < room::def[i].n_walls; j++) {
		uint pj = j ? j-1 : room::def[i].n_walls - 1;
		room::def[i].walls[j].build_side(&room::def[i].walls[pj]);
	}
	
	rata = new Rata(room::roompicker, vec(2, 2));
	rata->pickup_equip(new Item(NULL, rata->pos, item::white_dress));
	(new TileLayer(type::back_tiles))->activate();
	(new BulletLayer())->activate();
	(new TileLayer(type::front_tiles))->activate();
	(new CursorLayer())->activate();
	for (uint i=0; i < room::n_rooms; i++) {
		room::def[i].init();
	}

	rata->loc->enter();

	camera_jump = true;
	draw_hud = &hud_play;

	try { main_loop(); } catch (int x) { printf("Game quit succesfully with result %d.\n", x); }
	return 0;
}









