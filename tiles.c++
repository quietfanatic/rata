



// Maximum number of vertexes in a tile's physical shape
// Feel free to increase this value, but don't make it larger than necessary.

namespace tile {
	static const uint max_vertexes = 4;
	struct Def {
		const char* name;
		bool front;
		bool back;
		float friction;
		uint nvertexes;
		Vec vertexes [max_vertexes];
		FixProp* prop;
	};

	static const uint num_tiles = 9;
	static const Def def [num_tiles] = {
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











