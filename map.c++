
#ifdef HEADER

namespace map {
	struct Tile {
		uint16 tile1;
		uint16 tile2;
		Tile () :tile1(1), tile2(0) { }
	};

	const uint width = 128;
	const uint height = 128;

	Tile world [height][width];

	struct Pos {
		uint8 x;
		uint8 y;
		
		Pos () { }
		Pos (uint8 x, uint8 y) :x(x % width), y(y % height) { }
		Pos (Vec v) :x((int)floor(v.x) % width), y((int)floor(v.y) % height) { }
		Pos left () const { return Pos(x-1, y); }
		Pos right () const { return Pos(x+1, y); }
		Pos down () const { return Pos(x, y-1); }
		Pos up () const { return Pos(x, y+1); }

		bool operator == (Pos p) { return x == p.x && y == p.y; }
	};

	Tile& at (uint x, uint y) {
		return world[y % height][x % width];
	}
	Tile& at (Pos p) {
		return world[p.y][p.x];
	}

	void debug_print () {
		for (uint y=0; y < height; y++) {
			for (uint x=0; x < width; x++) {
				if (world[height-y-1][x].tile1)
					putchar('#');
				else
					putchar('.');
			}
			putchar('\n');
		}
	}

	void load_room (room::Def*);
	void unload_room (room::Def*);
}


#else

namespace map {

	 // Check if a point can be reached via jump (no obstacles)
	 // See notes
	bool can_reach_with_jump (Vec from, Vec vel, Vec to, float float_time) {
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




	void load_room (room::Def* r) {
		uint offset = 0;
		for (uint ry=0; ry < r->height; ry++)
		for (uint rx=0; rx < r->width; rx++) {
			uint x = rx + r->pos.x;
			uint y = ry + r->pos.y;
			uint16 tileid = r->tile(rx + offset, r->height - ry - 1);
			if (tileid == 0x8000) {  // Double tile
				at(x, y).tile1 = r->tile(rx + offset + 1, r->height - ry - 1);
				at(x, y).tile2 = r->tile(rx + offset + 2, r->height - ry - 1);
				offset += 2;
			}
			else at(x, y).tile1 = tileid;
		}
	}
	void unload_room (room::Def* r) {
		for (uint y=r->pos.y; y < r->pos.y + r->height; y++)
		for (uint x=r->pos.x; x < r->pos.x + r->width; x++)
			at(x, y) = Tile();
	}

}

#endif

