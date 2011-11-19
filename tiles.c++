
#ifdef HEADER


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

	static const uint width = 128;
	static const uint height = 128;

	Tile world [height][width];

	struct Pos {
		uint8 x;
		uint8 y;
		
		Pos () { }
		Pos(uint8 x, uint8 y) :x(x % width), y(y % height) { }
		Pos(Vec v) :x((int)floor(v.x) % width), y((int)floor(v.y) % height) { }
		inline Pos left () const { return Pos(x-1, y); }
		inline Pos right () const { return Pos(x+1, y); }
		inline Pos down () const { return Pos(x, y-1); }
		inline Pos up () const { return Pos(x, y+1); }

		inline bool operator == (Pos p) { return x == p.x && y == p.y; }
	};

	static inline Tile& at (uint8 x, uint8 y) {
		return world[y % height][x % width];
	}
	static inline Tile& at (Pos p) {
		return world[p.y][p.x];
	}

	Pos get_platform (Pos p) {
		while (at(p).nature != tile::platform && at(p).nature != tile::unknown) {
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
		return get_end_left(a) == get_end_left(b);
	}

	void debug_print () {
		for (uint y=0; y < height; y++) {
			for (uint x=0; x < width; x++) {
				putchar(world[height-y-1][x].nature);
			}
			putchar('\n');
		}
	}

	void load_room(Room*);
	void unload_room(Room*);
}


#else

namespace map {

	 // Check if a point can be reached via jump (no obstacles)
	 // See notes
	bool can_reach_with_jump(Vec from, Vec vel, Vec to, float float_time) {
		float t = (to.x - from.x) / vel.x;
		float height = 
		  t <= float_time
		    ?   vel.y                  * t
		      + (gravity/float_time/6) * t*t*t
		    :   vel.y*float_time
			  + (gravity/6)*float_time*float_time
		      + (vel.y + gravity/2*float_time*float_time) * (t-float_time)
		      + (gravity/2)                               * (t-float_time)*(t-float_time);
		return (to.y - from.y) < height;
	}




	void load_room (Room* room) {
		room::Desc* r = &room::desc[room->data];
		for (uint ry=0; ry < r->height; ry++)
		for (uint rx=0; rx < r->width; rx++) {
			uint x = rx + room->pos.x;
			uint y = ry + room->pos.y;
			at(x, y).id = r->tile(rx, r->height - ry - 1);
			at(x, y).nature = tile::def[abs(at(x, y).id)].nature;
			if (at(x, y).nature == tile::empty) {
				if (at(x, y-1).nature == tile::solid)
					at(x, y).nature = tile::platform;
				else if (at(x, y-1).nature == tile::unknown)
					at(x, y).nature = tile::unknown;
			}
		}
	}
	void unload_room (Room* room) {
		room::Desc* r = &room::desc[room->data];
		for (uint y=room->pos.y; y < room->pos.y + r->height; y++)
		for (uint x=room->pos.x; x < room->pos.x + r->width; x++)
			at(x, y) = Tile();
	}

}


#endif


