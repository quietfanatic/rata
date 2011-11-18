

#ifdef HEADER

const uint SAVEFILE_VERSION = 3;

const uint n_saved_things = 8;
extern obj::Desc saved_things [n_saved_things];

#else

obj::Desc saved_things [n_saved_things] = {
	obj::Desc(room::roompicker, obj::rata, Vec(10,10)),
	obj::Desc(room::everywhere, obj::tiles_back, Vec(0,0), Vec(0,0), 0),
	obj::Desc(room::everywhere, obj::bullet_layer),
	obj::Desc(room::everywhere, obj::tiles_front, Vec(0,0), Vec(0,0), 1),
	obj::Desc(room::everywhere, obj::lifebar),
	obj::Desc(room::everywhere, obj::shade, Vec(0,0), Vec(20,15), -1, 0x0f0f0f4f),
	obj::Desc(room::equipment, obj::item, Vec(0,0), Vec(0,0), 0, item::white_dress),
	obj::Desc(room::roompicker, obj::item, Vec(14.5, 5.0), Vec(0,0), 0, item::handgun),
//	obj::Desc(room::test2, obj::item, Vec(8.5, 21.0), Vec(0,0), 0, item::white_dress),
//	obj::Desc(room::test2, obj::item, Vec(38.5, 17.0), Vec(0,0), 0, item::handgun),
//	obj::Desc(room::test4, obj::item, Vec(21.0, 1.0), Vec(0,0), 0, item::handgun),
//	obj::Desc(room::test4, obj::item, Vec(28.0, 1.0), Vec(0,0), 0, item::white_dress),
//	obj::Desc(room::test4, obj::item, Vec(25.0, 1.0), Vec(0,0), 0, item::boots),
//	obj::Desc(room::test4, obj::item, Vec(27.0, 14.0), Vec(0,0), 0, item::helmet),
};


static inline bool readthing (FILE* f, uint32* x) {
	uint8 b [4];
	if (fread(b, 4, 1, f)) {
		*x = b[0] << 24
		   | b[1] << 16
		   | b[2] << 8
		   | b[3];
		return true;
	}
	printf("Error: Could not read save file!\n");
	return false;
}
static inline bool writething (FILE* f, uint32 x) {
	uint8 b [4];
	b[0] = x >> 24;
	b[1] = x >> 16;
	b[2] = x >> 8;
	b[3] = x;
	if (fwrite(b, 4, 1, f)) return true;
	printf("Error: Could not write to save file!\n");
	return false;
}

void load_save () {
	FILE* f = fopen("savefile", "r");
	if (f) {
		uint d;
		readthing(f, &d);
		if (d != 0) printf("Error: Save file appears to be corrupted.\n");
		readthing(f, &d);
		if (d != SAVEFILE_VERSION) printf("Error: Save file is of the wrong version.\n");
		for (uint i=0; i < n_saved_things; i++) {
			uint roomandid;
			readthing(f, &roomandid);
			saved_things[i].room = roomandid >> 16;
			saved_things[i].id = roomandid;
			readthing(f, (uint32*)&saved_things[i].pos.x);
			readthing(f, (uint32*)&saved_things[i].pos.y);
			readthing(f, (uint32*)&saved_things[i].vel.x);
			readthing(f, (uint32*)&saved_things[i].vel.y);
			readthing(f, (uint32*)&saved_things[i].facing);
			readthing(f, &saved_things[i].data);
			readthing(f, &saved_things[i].data2);
		}
		fclose(f);
	}
	else {
		printf("Could not open save file; creating new game.\n");
	}
	for (uint i=0; i < n_saved_things; i++) {
		if (saved_things[i].id == obj::rata) {
			room::currenti = saved_things[i].room;
		}
		if (saved_things[i].room == room::everywhere) {
			saved_things[i].manifest();
		}
		else if (saved_things[i].room == room::equipment) {
//			if (item::def[saved_things[i].data].slot >= 0)
//				rata->equipment[item::def[saved_things[i].data].slot] = &saved_things[i];
//			if (item::def[saved_things[i].data].otherslot >= 0)
//				rata->equipment[item::def[saved_things[i].data].otherslot] = &saved_things[i];
		}
		else if (saved_things[i].room <= -100) {
//			rata->inventory[-(saved_things[i].room + 100)] = &saved_things[i];
		}
	}
}


void save_save () {
	FILE* f = fopen("savefile", "w");
	if (f) {
		writething(f, 0);
		writething(f, SAVEFILE_VERSION);
		for (uint i=0; i < n_saved_things; i++) {
			uint32 roomandid = (uint32)(uint16)saved_things[i].room << 16 | (uint16)saved_things[i].id;
			writething(f, roomandid);
			writething(f, *(uint32*)&saved_things[i].pos.x);
			writething(f, *(uint32*)&saved_things[i].pos.y);
			writething(f, *(uint32*)&saved_things[i].vel.x);
			writething(f, *(uint32*)&saved_things[i].vel.y);
			writething(f, saved_things[i].facing);
			writething(f, saved_things[i].data);
			writething(f, saved_things[i].data2);
		}
		fclose(f);
		printf("Game has been saved.\n");
	}
	else {
		printf("Error: Could not open save file for writing!\n");
	}
}







#endif

