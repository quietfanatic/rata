#define MAPEDITOR

#include "top.c++"


char* room_name = NULL;

bool load_room (room::Room* newroom) {
	if (!newroom) return false;
	room::current = newroom;
	float w = room::current->width/2;
	float h = room::current->height/2;
	window_view.SetFromRect(sf::FloatRect(
		-30/window_scale + w, 22.5/window_scale + h,
		30/window_scale + w, -22.5/window_scale + h
	));
	return true;
}

void save_room () {
	char filename [
		  strlen("rooms/")
		+ strlen(room_name)
		+ strlen(".room.c++")
		+ 1
	];
	filename[0] = 0;
	strcat(filename, "rooms/");
	strcat(filename, room_name);
	strcat(filename, ".room.c++");
	FILE* F = fopen(filename, "w");
	if (!F) printf("Error: Failed to open %s for writing.\n", filename);
	if (room::current->print_to_file(F)) {
		printf("Saved room to %s.\n", filename);
	}
	else {
		printf("Error: Failed to write to %s.\n", filename);
	}
	system("perl gen_roomlist.pl > roomlist.c++");
}


int main (int argc, char** argv) {
	if (argc == 1) {
		printf("Error: Must be given room name as argument.\n");
		return 1;
	}
	mapeditor = true;
	room_name = argv[1];
	load_img();
	if (!load_room( room::name(room_name) )) {
		printf("Error: No room %s found.\n", argv[1]);
		return 1;
	}
	printf("Editing room %s.\n", argv[1]);
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
		obj::roomsettings
	).manifest();
	obj::Desc(
		obj::editor_menu
	).manifest();
	obj::Desc(
		obj::clickable_text,
		(void*)" Save ",
		0, 45/window_scale, 0, 0,
		0, false,
		(uint32)(void*)&save_room
	).manifest();
	char roomsize_text[32];
	sprintf(roomsize_text, "Size: %dx%d", room::current->width, room::current->height);
	Object* roomsize_obj = obj::Desc(
		obj::clickable_text,
		(void*)roomsize_text,
		viewwidth()-4, viewheight()-32*PX/window_scale, 0, 0,
		0, false,
		(uint32)NULL
	).manifest();


	try { main_loop(); } catch (int x) { printf("Editor successfully quit with result %d\n", x); }
}


