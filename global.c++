

extern const uint obj::n_globals = 7;
obj::Desc obj::saved [obj::n_globals] = {

	{0, obj::game, Vec(0, 0), Vec(0, 0), 0, 0, 0},
	{2, obj::rata, Vec(2, 2), Vec(0, 0), 0, 0, 0},
	{0, obj::room, Vec(0, 0), Vec(0, 0), 0, room::roompicker, 0},
	{-1, obj::back_tiles, Vec(0, 0), Vec(0, 0), 0, 0, 0},
	{-1, obj::bullet_layer, Vec(0, 0), Vec(0, 0), 0, 0, 0},
	{-1, obj::front_tiles, Vec(0, 0), Vec(0, 0), 1, 0, 0},
	{-1, obj::shade, Vec(0, 0), Vec(0, 0), 0, 0x0000004f, 0}



};


Actor* obj::global [obj::n_globals];


void init_objects () {
	for (uint i=0; i < obj::n_globals; i++) {
		obj::global[i] = obj::saved[i].manifest();
	}
	for (uint i=0; i < obj::n_globals; i++) {
		if (obj::saved[i].loc > -1)
			obj::global[obj::saved[i].loc]->take(obj::global[i]);
	}
	for (uint i=0; i < obj::n_globals; i++) {
		if (obj::saved[i].loc < 0)
			obj::global[i]->activate();
	}
	obj::global[obj::global[1]->loc]->activate();  // Rata's room
}










