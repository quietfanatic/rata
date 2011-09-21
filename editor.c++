

#include "top.c++"




void load_room (room::Room* newroom) {
	room::current = newroom;
	float w = room::current->width/2*UNPX;
	float h = room::current->height/2*UNPX;
	window_view.SetFromRect(sf::FloatRect(
		-480/window_scale + w, -360/window_scale - h,
		480/window_scale + w, 360/window_scale - h
	));
}


int main () {
	mapeditor = true;
	load_img();
	 // Set video
	window = new sf::RenderWindow;
	window->Create(sf::VideoMode(960, 720, 32), "");

	obj::Desc(
		obj::tilemap_editor
	).manifest();

	load_room(&room::file::test1::room);

	try { main_loop(); } catch (int x) { printf("Editor successfully quit with result %d\n", x); }
}


