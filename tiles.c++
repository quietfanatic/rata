



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

	static const uint width = 128;
	static const uint height = 128;

	struct Tile {
		int16 id;
		int16 nature;
		Tile () :id(1), nature(tile::unknown) { }
	};
	Tile world [height][width];

	struct Pos {
		uint8 x;
		uint8 y;

		Pos(uint8 x, uint8 y) :x(x % width), y(y % height) { }
		Pos(Vec v) :x((uint8)v.x % width), y((uint8)v.y % height) { }
		inline Pos left () const { return Pos(x-1, y); }
		inline Pos right () const { return Pos(x+1, y); }
		inline Pos down () const { return Pos(x, y-1); }
		inline Pos up () const { return Pos(x, y+1); }
	};



	static inline Tile at (uint8 x, uint8 y) {
		return world[y % height][x % width];
	}
	static inline Tile at (Pos p) {
		return world[p.y][p.x];
	}

	Pos get_platform (Pos p) {
		if (at(p).nature == tile::unknown) return p;
		while (at(p).nature != tile::platform) {
			p = p.down();
		}
		return p;
	}

	Pos get_end_left (Pos p) {
		while (at(p).nature == tile::platform) {
			p = p.left();
		}
		return p.right();
	}
	Pos get_end_right (Pos p) {
		while (at(p).nature == tile::platform) {
			p = p.right();
		}
		return p.left();
	}
	static inline bool same_platform (Pos a, Pos b) {
		b = get_end_left(b);
		return a.x >= b.x && a.y == b.y;
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





