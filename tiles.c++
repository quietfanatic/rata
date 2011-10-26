



// Maximum number of vertexes in a tile's physical shape
// Feel free to increase this value, but don't make it larger than necessary.

namespace tile {
	static const uint max_vertexes = 4;
	struct Def {
		const char* name;
		int16 nature;
		bool front;
		bool back;
		float friction;
		uint nvertexes;
		Vec vertexes [max_vertexes];
		FixProp* prop;
	};
	enum {
		empty,
		solid,
		unknown
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
		int16 height;  // above standable ground, in tiles
		 // height is -1 if solid, -2 if unknown
		Tile () :id(1), height(-2) { }
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
			switch (tile::def[world[y][x].id].nature) {
				case tile::unknown: {
					world[y][x].height = -2; break;
				}
				case tile::solid: {
					world[y][x].height = -1; break;
				}
				case tile::empty: {
					int lower = at(x, y-1).height;
					world[y][x].height = (lower == -2 ? -2 : lower+1);
					break;
				}
			}
		}
	}

	void debug_print () {
		for (uint y=0; y < height; y++) {
			for (uint x=0; x < width; x++) {
				putchar(MIN('0' + world[height-y-1][x].height, '~'));
			}
			putchar('\n');
		}
	}

}





