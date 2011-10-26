



#ifdef DEF_ONLY

// Maximum number of vertexes in a tile's physical shape
// Feel free to increase this value, but don't make it larger than necessary.
#define TILE_MAX_VERTEXES 4

struct Tileinfo {
	const char* name;
	bool front;
	bool back;
	float friction;
	uint nvertexes;
	Vec vertexes [TILE_MAX_VERTEXES];
	FixProp* prop;
};


extern uint num_tiles;
extern Tileinfo tileinfo [];
#else


#define v Vec



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

uint num_tiles = 9;  // <-- This number must equal the number of tiles below
Tileinfo tileinfo [] = {
	{"Empty", 0, 0, 0.0, 0},
	{"Metal block", 1, 0, 0.4, 4, {v(0, 0), v(1, 0), v(1, 1), v(0, 1)}, NULL},
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









#endif


