


#ifdef DEF_ONLY
struct Tileinfo {
	const char* name;
	bool front;
	bool back;
	float friction;
	uint nvertexes;
	b2Vec2 vertexes [4];
};

extern uint num_tiles;
extern Tileinfo tileinfo [];
#else


#define v b2Vec2

uint num_tiles = 9;
Tileinfo tileinfo [] = {
	{"Empty", 0, 0, 0.0, 0,},
	{"Metal block", 1, 0, 0.4, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Metal support", 0, 1, 0.0, 0},
	{"Metal support", 0, 1, 0.0, 0},
	{"Metal support", 0, 1, 0.0, 0},
	{"Metal support", 0, 1, 0.0, 0},
	{"Metal support", 0, 1, 0.0, 0},
	{"Metal ramp l", 1, 0, 1.5, 3, {v(0, 0), v(1, 0), v(1, 1)}},
	{"Metal ramp r", 1, 0, 1.5, 3, {v(0, 0), v(1, 0), v(0, 1)}},
	{"END OF TILES", 0, 0, 9}
};

#undef v


/*

Here is our tile-to-object algorithm.
- Each tile starts out with a number of edges (four for a plain solid tile).
- Those edge will have recorded not only their endpoints, but also those edges
   to which they are attached (for a plain solid tile, cyclicly in a square).
- The edges will be sorted by position.
- Any two edges that are deemed close enough will be cancelled out, with their
   neighbors becoming attached to each other.

*/

struct TileEdge {
	bool use;
	TileEdge* n1;
	b2Vec2 v1;
	b2Vec2 v2;
	TileEdge* n2;
};







#endif


