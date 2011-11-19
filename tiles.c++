

namespace tile {
	static const uint max_vertexes = 4;
	struct Def {
		const char* name;
		int8 nature;
		bool front;
		bool back;
		float friction;
		uint nvertexes;
		Vec vertexes [max_vertexes];
		FixProp* prop;
	};
	enum {
		empty = ' ',
		platform = '_',
		solid = '#',
		unknown = '.',
	};

	static const uint num_tiles = 9;
	static const Def def [num_tiles] = {

		{"Empty", empty, 0, 0, 0.0, 0},
		{"Metal block", solid, 1, 0, 0.4, 4, {Vec(0, 0), Vec(1, 0), Vec(1, 1), Vec(0, 1)}, NULL},
		{"Metal support", empty, 0, 1, 0.0, 0},
		{"Metal support", empty, 0, 1, 0.0, 0},
		{"Metal support", empty, 0, 1, 0.0, 0},
		{"Metal support", empty, 0, 1, 0.0, 0},
		{"Metal support", empty, 0, 1, 0.0, 0},
		{"Metal ramp l", unknown, 1, 0, 1.5, 3, {Vec(0, 0), Vec(1, 0), Vec(1, 1)}},
		{"Metal ramp r", unknown, 1, 0, 1.5, 3, {Vec(0, 0), Vec(1, 0), Vec(0, 1)}},

	};
}


