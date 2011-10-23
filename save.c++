

#ifdef DEF_ONLY

const uint n_saved_things = 9;
extern obj::Desc saved_things [n_saved_things];

#else

obj::Desc saved_things [n_saved_things] = {
	obj::Desc(-200, obj::item, Vec(0,0), Vec(0,0), 0, item::white_dress),
	obj::Desc(room::roompicker, obj::item, Vec(14.5, 5.0), Vec(0,0), 0, item::handgun),
	obj::Desc(room::test2, obj::item, Vec(8.5, 21.0), Vec(0,0), 0, item::white_dress),
	obj::Desc(room::test2, obj::item, Vec(38.5, 17.0), Vec(0,0), 0, item::handgun),
	obj::Desc(room::test4, obj::item, Vec(21.0, 1.0), Vec(0,0), 0, item::handgun),
	obj::Desc(room::test4, obj::item, Vec(28.0, 1.0), Vec(0,0), 0, item::white_dress),
	obj::Desc(room::test4, obj::item, Vec(25.0, 1.0), Vec(0,0), 0, item::boots),
	obj::Desc(room::test4, obj::item, Vec(27.0, 14.0), Vec(0,0), 0, item::helmet),
};




void load_save () {
	FILE* f = fopen("savefile", "r");
	if (f) {
		fclose(f);
	}
	else {
		printf("Could not open save file; creating new game.\n");
	}
	for (uint i=0; i < n_saved_things; i++) {
		if (saved_things[i].room == -200) {
			if (item::def[saved_things[i].data].slot >= 0)
				rata->equipment[item::def[saved_things[i].data].slot] = &saved_things[i];
			if (item::def[saved_things[i].data].otherslot >= 0)
				rata->equipment[item::def[saved_things[i].data].otherslot] = &saved_things[i];
		}
		else if (saved_things[i].room <= -100) {
			rata->inventory[-(saved_things[i].room + 100)] = &saved_things[i];
		}
	}
}


void save_save () {

}







#endif

