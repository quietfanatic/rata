

namespace room {
	struct Desc {
		uint16 width;
		uint16 height;
		const int16* tiles;
		uint32 nneighbors;
		const int16* neighbors;
		uint32 walls;
		
		int16 tile (uint x, uint y) {
			return tiles[y * width + x];
		}
	};
	enum Location {
		wherever = -1,
		temp = -2,
		equipment = -3,
		everywhere = -4
	};
	extern Desc desc [];



#define ROOM_BEGIN
#define ROOM_WIDTH(...) static const uint16 width = __VA_ARGS__;
#define ROOM_HEIGHT(...) static const uint16 height = __VA_ARGS__;
#define ROOM_TILES(...) const int16 tiles [width * height] = {__VA_ARGS__};
#define ROOM_NNEIGHBORS(...) static const uint32 nneighbors = __VA_ARGS__;
#define ROOM_NEIGHBORS(...) const int16 neighbors [nneighbors] = {__VA_ARGS__};
#define ROOM_WALLS(...) const uint32 walls = __VA_ARGS__;
#define ROOM_END static const room::Desc desc = {width, height, tiles, nneighbors, neighbors, walls};



#include "roomlist.c++"


}











