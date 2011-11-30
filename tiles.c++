

namespace tile {
	const uint max_vertexes = 4;
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

	const uint num_tiles = 9;
	const Def def [num_tiles] = {

		{"Empty", empty, 0, 0, 0.0, 0},
		{"Metal block", solid, 1, 0, 0.4, 4, {{0, 0}, {1, 0}, {1, 1}, {0, 1}}, NULL},
		{"Metal support", empty, 0, 1, 0.0, 0},
		{"Metal support", empty, 0, 1, 0.0, 0},
		{"Metal support", empty, 0, 1, 0.0, 0},
		{"Metal support", empty, 0, 1, 0.0, 0},
		{"Metal support", empty, 0, 1, 0.0, 0},
		{"Metal ramp l", unknown, 1, 0, 1.5, 3, {{0, 0}, {1, 0}, {1, 1}}},
		{"Metal ramp r", unknown, 1, 0, 1.5, 3, {{0, 0}, {1, 0}, {0, 1}}},

	};
}


