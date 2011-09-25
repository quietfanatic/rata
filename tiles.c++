



#ifdef DEF_ONLY

// Maximum number of vertexes in a tile's physical shape
// Feel free to increase this value, but don't make it larger than necessary.
#define TILE_MAX_VERTEXES 8

struct Tileinfo {
	const char* name;
	bool front;
	bool back;
	float friction;
	uint nvertexes;
	b2Vec2 vertexes [TILE_MAX_VERTEXES];
};

extern uint num_tiles;
extern Tileinfo tileinfo [];
#else


#define v b2Vec2



// The format for a tile entry is: {<name>, <front>, <back>, <friction>, <nvertexes>, <vertexes>}
// where: <name> is the name of the tile (The program doesn't current use this).
//        <front> is 1 if the tile is drawn in front of the player and enemies
//        <back> is 1 if the tile is drawn behind the player and enemies
//        <friction> is the friction coefficient for the tile.  Some useful values are:
//                  0.4: moderate friction
//                  0.5: A surface with high traction
//                  0.2, 0.1: A slippery surface
//                  1.5: A 45-degree ramp that can be walked on
//                  0.5: A 45-degree ramp that will make you slip
//                        (In general, in order to stand on steeper slopes, more friction is needed)
//        <nvertexes> is the number of vertexes (the size of <vertexes> below
//        <vertexes> is the list of vertexes that compose the physical shape of the tile.

uint num_tiles = 48;  // <-- This number must equal the number of tiles below
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
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},

	{"Dark lab tile ltr", 1, 0, 0.5, 6, {v(0, 0), v(1, 0), v(1, 0.875), v(0.875, 1), v(0.125, 1), v(0, 0.875)}},
	{"Dark lab tile ltb", 1, 0, 0.5, 6, {v(0.125, 0), v(1, 0), v(1, 1), v(0.125, 1), v(0, 0.875), v(0, 0.125)}},
	{"Dark lab tile lt+c", 1, 0, 0.5, 5, {v(0, 0), v(1, 0), v(1, 1), v(0.125, 1), v(0.825, 0)}},
	{"Dark lab tile t+c", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile ilt+c", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile l+ct", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile l+c", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile lt", 1, 0, 0.5, 5, {v(0, 0), v(1, 0), v(1, 1), v(0.125, 1), v(0.825, 0)}},
	{"Dark lab tile t", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile ilt", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile l", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile bg l", 1, 0, 0, 0,},
	{"Dark lab tile bg r", 1, 0, 0, 0,},
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},

	{"Dark lab tile lbr", 1, 0, 0.5, 6, {v(0.125, 0), v(0.875, 0), v(1, 0.125), v(1, 1), v(0, 1), v(0, 0.125)}},
	{"Dark lab tile ltbr", 1, 0, 0.5, 8, {v(0.125, 0), v(0.875, 0), v(1, 0.125), v(1, 0.875), v(0.875, 1), v(0.125, 1), v(0, 0.875), v(0, 0.125)}},
	{"Dark lab tile lb+c", 1, 0, 0.5, 5, {v(0.125, 0), v(1, 0), v(1, 1), v(0, 1), v(0, 0.125)}},
	{"Dark lab tile b+c", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile ilb+c", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile l+cb", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile +c", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile lb", 1, 0, 0.5, 5, {v(0.125, 0), v(1, 0), v(1, 1), v(0, 1), v(0, 0.125)}},
	{"Dark lab tile b", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile ilb", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile", 1, 0, 0.5, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}},
	{"Dark lab tile bg", 1, 0, 0, 0,},
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},
	{"Nothing", 0, 0, 0,},


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


