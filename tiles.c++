

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
	 // tile types
	enum {
		empty = ' ',
		platform = '_',
		solid = '#',
		unknown = '.',
	};
	
	 // A tile entry is:
	 //   {name, type, front, back, nvertexes, vertexes},
	 // where name is a string,
	 //       type is one of the above types (this doesn't actually matter right now)
	 //       front, if 1, means the tile is drawn in foreground
	 //       back, if 1, means the tile is drawn in background
	 //       nvertexes is the size of 'vertexes'
	 //       vertexes is a list of vectors describing the corners of the tile,
	 //        in counter-clockwise order.  X is to the left, Y is up.
	const uint num_tiles = 9;  // Make sure this number is correct
	const Def def [num_tiles] = {

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


