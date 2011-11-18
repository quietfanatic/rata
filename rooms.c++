
#ifdef HEADER

#ifdef MAPEDITOR
#define roomconst
#else
#define roomconst const
#endif

namespace room {
	int current = -1;
	struct Room {
		roomconst Vec pos;
		roomconst uint16 width;
		roomconst uint16 height;
		roomconst int16* tiles;
		roomconst uint32 nneighbors;
		roomconst int32* neighbors;
		roomconst uint32 walls;
		roomconst uint32 nobjects;
		roomconst obj::Desc* objects;
		obj::Desc* saved_objects;
		int16 id;
		bool loaded;
		
		void enter ();
		void unload ();
		void load ();
		int16 tile (uint x, uint y);
		void manifest_geometry ();
	};
	enum Location {
		wherever = -1,
		temp = -2,
		equipment = -3,
		everywhere = -4
	};
	extern Room* list [];
}
typedef room::Room Room;

#else

namespace room {
	void Room::enter () {
		if (current != id) {
			Room* old = room::list[current];
			 // Unload old neighbors that aren't our neighbors.
			for (uint i=0; i < old->nneighbors; i++) {
				if (room::list[old->neighbors[i]]->loaded)
				if (old->neighbors[i] != id) {
					for (uint j=0; j < nneighbors; j++) {
						if (old->neighbors[i] == neighbors[j])
							goto keep_this_one;
					}
					 // This room is obsolete
					room::list[old->neighbors[i]]->unload();
				}
				keep_this_one: { }
			}
		}
		current = id;
		if (!loaded) load();
		 // Load neighbors
		for (uint i=0; i < nneighbors; i++)
		if (!room::list[neighbors[i]]->loaded)
			room::list[neighbors[i]]->load();
	}
	void Room::load () {
		loaded = true;
		if (!saved_objects) {
			saved_objects = (obj::Desc*)malloc(nobjects*sizeof(obj::Desc));
			memcpy((void*)saved_objects, (void*)objects, nobjects*sizeof(obj::Desc));
		}
		(new obj::Desc(
			-2, obj::tilemap,
			pos, Vec(0, 0), 0,
			id
		))->manifest();
		map::load_room(id);
		 // Load these objects
		for (uint i = 0; i < nobjects; i++)
		if (saved_objects[i].id > 0)
			saved_objects[i].manifest();
		for (uint i=0; i < n_saved_things; i++)
		if (saved_things[i].room == id)
		if (saved_things[i].id > 0)
			saved_things[i].manifest();
//		if (bgm > -1 && bgm != bgm::current) {
//			bgm::current = bgm;
//			bgm::music.OpenFromFile(bgm::name[bgm]);
//			bgm::music.SetLoop(true);
//			bgm::music.SetVolume(80);
//			bgm::music.Play();
//		}
	}

	void Room::unload () {
		loaded = false;
		for (Actor* a = actors_by_depth; a; a = a->next_depth) {
			if (a->desc->room == current) {
				a->destroy();
			}
		}
		map::unload_room(id);
	}

	int16 Room::tile (uint x, uint y) {
		return tiles[y * width + x];
	}


#define ROOM_BEGIN
#define ROOM_POS(x, y) static const Vec pos = Vec(x, y);
#define ROOM_WIDTH(...) static const uint16 width = __VA_ARGS__;
#define ROOM_HEIGHT(...) static const uint16 height = __VA_ARGS__;
#define ROOM_TILES(...) roomconst int16 tiles [width * height] = {__VA_ARGS__};
#define ROOM_NNEIGHBORS(...) static const uint32 nneighbors = __VA_ARGS__;
#define ROOM_NEIGHBORS(...) roomconst int32 neighbors [nneighbors] = {__VA_ARGS__};
#define ROOM_WALLS(...) roomconst uint32 walls = __VA_ARGS__;
#define ROOM_NOBJECTS(...) static const uint32 nobjects = __VA_ARGS__;
#define ROOM_OBJECTS(...) roomconst obj::Desc objects [nobjects] = {__VA_ARGS__};
#define ROOM_OBJECT(id, x, y, xvel, yvel, facing, data, data2) obj::Desc(-1, id, Vec(x, y), Vec(xvel, yvel), facing, data, data2),
#define ROOM_END Room room = {pos, width, height, tiles, nneighbors, neighbors, walls, nobjects, objects, NULL, THIS_ROOM, false};



#include "roomlist.c++"


}











#endif
