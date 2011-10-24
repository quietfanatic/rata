#define MAPEDITOR

#include "top.c++"


char* room_name = NULL;

bool load_room (int16 newroom) {
	room::currenti = newroom;
	room::current = room::list[newroom];
	float w = room::current->width/2;
	float h = room::current->height/2;
	window_view.SetFromRect(sf::FloatRect(
		-30/window_scale + w, 22.5/window_scale + h,
		30/window_scale + w, -22.5/window_scale + h
	));
	return true;
}

void save_room () {
	FILE* F = fopen(room::filename[room::currenti], "w");
	if (!F) printf("Error: Failed to open %s for writing.\n", room::filename[room::currenti]);
	if (room::current->print_to_file(F)) {
		printf("Saved room to %s.\n", room::filename[room::currenti]);
	}
	else {
		printf("Error: Failed to write to %s.\n", room::filename[room::currenti]);
	}
	system("perl gen_roomlist.pl > roomlist.c++");
}


int main (int argc, char** argv) {
	if (argc == 1) {
		printf("Error: Must be given room number as argument.\n");
		return 1;
	}
	mapeditor = true;
	uint room_no = atof(argv[1]);
	img::load_img();
	if (room_no > room::n_rooms) {
		printf("Error: No room numer %u\n", room_no);
		return 1;
	}
	else {
		load_room(room_no);
	}
	printf("Editing room %u.\n", room_no);
	 // Set video
	window = new sf::RenderWindow;
	window->Create(sf::VideoMode(960, 720, 32), "");

	obj::Desc(
		-1, obj::tilemap_editor,
		Vec(0, 0), Vec(30, 22.5)
	).manifest();
	obj::Desc(
		-1, obj::tilepicker,
		Vec(0, 0), Vec(1+4*PX, 21.5)
	).manifest();
	obj::Desc(
		-1, obj::roomsettings,
		Vec(26-2*PX, 0), Vec(4+2*PX, 21.5)
	).manifest();
	obj::Desc(
		-1, obj::editor_menu,
		Vec(0, 21.5), Vec(30, 1)
	).manifest();
	obj::Desc(
		-1, obj::clickable_text,
		Vec(0, 21.5), Vec(4, 1), 0,
		(uint32)(char*)"Save",
		(uint32)(void*)&save_room
	).manifest();

	char roomsize_text[32];
	sprintf(roomsize_text, "Size: %dx%d", room::current->width, room::current->height);
	Object* roomsize_obj = obj::Desc(
		-1, obj::clickable_text,
		Vec(26, 20.5), Vec(4, 1), 0,
		(uint32)(void*)roomsize_text,
		(uint32)NULL
	).manifest();


	try { main_loop(); } catch (int x) { printf("Editor successfully quit with result %d\n", x); }
}


