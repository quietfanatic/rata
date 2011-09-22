#define MAPEDITOR

#include "top.c++"




void load_room (room::Room* newroom) {
	room::current = newroom;
	float w = room::current->width/2;
	float h = room::current->height/2;
	window_view.SetFromRect(sf::FloatRect(
		-30/window_scale + w, 22.5/window_scale + h,
		30/window_scale + w, -22.5/window_scale + h
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
	obj::Desc(
		obj::tilepicker
	).manifest();
	obj::Desc(
		obj::clickable_text,
		(void*)" Test text ",
		0, 45/window_scale, 0, 0, NULL
	).manifest();

	load_room(&room::file::test1::room);

	try { main_loop(); } catch (int x) { printf("Editor successfully quit with result %d\n", x); }
}


