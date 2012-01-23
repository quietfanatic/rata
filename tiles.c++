

namespace tile {
	const uint max_vertexes = 4;
	struct Def {
		const char* name;
		bool front;
		bool back;
		float friction;
		uint nvertexes;
		Vec vertexes [max_vertexes];
		FixProp* prop;
	};
	
	 // TILE CATALOGUE
	 // A tile entry is:
	 //   {name, front, back, friction, nvertexes, vertexes},
	 // where name is a the name of the tile (currently unused, intended for easy reading)
	 //       front, if 1, means the tile is drawn in foreground
	 //       back, if 1, means the tile is drawn in background
	 //       friction is the amount of friction of the material, from 0 to whatever
	 //        0.4 is pretty normal.  Something slippery should be 0.1 or something
	 //       nvertexes is the size of 'vertexes'
	 //       vertexes is a list of vectors describing the corners of the tile,
	 //        in counter-clockwise order.  X is to the left, Y is up.
	const uint num_tiles = 9;  // Make sure this number is correct
	const Def def [num_tiles] = {

		{"Empty", 0, 0, 0.0, 0},
		{"Metal block", 1, 0, 0.4, 4, {Vec(0, 0), Vec(1, 0), Vec(1, 1), Vec(0, 1)}, NULL},
		{"Metal support", 0, 1, 0.0, 0},
		{"Metal support", 0, 1, 0.0, 0},
		{"Metal support", 0, 1, 0.0, 0},
		{"Metal support", 0, 1, 0.0, 0},
		{"Metal support", 0, 1, 0.0, 0},
		{"Metal ramp l", 1, 0, 1.5, 3, {Vec(0, 0), Vec(1, 0), Vec(1, 1)}},
		{"Metal ramp r", 1, 0, 1.5, 3, {Vec(0, 0), Vec(1, 0), Vec(0, 1)}},

	};
}


