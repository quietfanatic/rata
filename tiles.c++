


#ifdef DEF_ONLY
struct Tileinfo {
	const char* name;
	bool front;
	bool back;
	uint nvertexes;
	b2Vec2 vertexes [4];
};

extern Tileinfo tileinfo [];
#else


#define v b2Vec2

Tileinfo tileinfo [] = {
	{"Empty", 0, 0, 0,},
	{"Metal block", 0, 1, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Metal support", 0, 1, 0},
	{"Metal support", 0, 1, 0},
	{"Metal support", 0, 1, 0},
	{"Metal support", 0, 1, 0},
	{"Metal support", 0, 1, 0},
	{"END OF TILE", 0, 0, 9}
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


