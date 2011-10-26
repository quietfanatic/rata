



// Maximum number of vertexes in a tile's physical shape
// Feel free to increase this value, but don't make it larger than necessary.

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


namespace map {

	struct Tile {
		int16 id;
		int16 nature;
		Tile () :id(1), nature(tile::unknown) { }
	};

	struct Platform {
		int8 left_x;
		int8 left_y;
		int8 right_x;
		int8 right_y;
	};

	static const uint width = 128;
	static const uint height = 128;

	Tile world [height][width];
	static inline Tile at (int x, int y) {
		return world[y % height][x % width];
	}
	static inline Tile at (Vec v) {
		return at(floor(v.x), floor(v.y));
	}
	void clear () {
		for (uint y=0; y < height; y++)
		for (uint x=0; x < width; x++)
			world[y][x] = Tile();
	}
	void load_room (room::Room* r) {
		for (uint y=0; y < r->height; y++)
		for (uint x=0; x < r->width; x++) {
			world[y][x].id = r->tile(x, r->height-y-1);
			world[y][x].nature = tile::def[abs(world[y][x].id)].nature;
			if (world[y][x].nature == tile::empty) {
				if (world[y-1][x].nature == tile::solid)
					world[y][x].nature = tile::platform;
				else if (world[y-1][x].nature == tile::unknown)
					world[y][x].nature = tile::unknown;
			}
		}
	}

	void debug_print () {
		for (uint y=0; y < height; y++) {
			for (uint x=0; x < width; x++) {
				putchar(world[height-y-1][x].nature);
			}
			putchar('\n');
		}
	}

}





