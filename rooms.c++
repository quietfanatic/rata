#ifdef HEADER
struct Spatial;


namespace room {
	struct Def {
		Vec pos;
		uint16 width;
		uint16 height;
		const int16* tiles;
		uint32 n_neighbors;
		room::Def*const* neighbors;
		uint32 n_walls;
		Wall* walls;
		void (& init ) ();
		bool loaded;
		Spatial* contents;

		int16 tile (uint x, uint y) {
			return tiles[y * width + x];
		}
		void receive (Spatial* a);
		void release (Spatial* a);
		void load ();
		void unload ();
		bool is_neighbor (room::Def* r);
		void enter ();
	};
	extern Def def [];

}

#else

namespace room {
	void Def::receive (Spatial* a) {
		if (a->loc) a->loc->release(a);
		a->loc = this;
		printf("Receiving a Spatial.\n");
		if (contents) contents->prev = a;
		a->next = contents;
		a->prev = NULL;
		contents = a;
		if (loaded) {
			if (!a->active && !a->awaiting_activation)
				a->activate();
		}
		else {
			if (a->active)
				a->deactivate();
		}
	}
	void Def::release (Spatial* a) {
		if (a->loc->contents == a)
			a->loc->contents = a->next;
		if (a->next) a->next->prev = a->prev;
		if (a->prev) a->prev->next = a->next;
	}
	void Def::load () {
		loaded = true;
		map::load_room(this);
		for (Spatial* a = contents; a; a = a->next)
		if (a->type > -1)
		if (!a->active && !a->awaiting_activation) {
			a->activate();
		}
	}
	void Def::unload () {
		loaded = false;
		for (Spatial* a = contents; a; a = a->next)
		if (a->type > -1)
		if (a->active)
			a->deactivate();
		map::unload_room(this);
	}
	bool Def::is_neighbor (room::Def* r) {
		if (r == this) return true;
		for (uint i=0; i < n_neighbors; i++)
		if (r == neighbors[i]) return true;
		return false;
	}
	void Def::enter () {
		if (current_room) {
			if (current_room->loaded)
			if (!is_neighbor(current_room))
				current_room->unload();
			for (uint i=0; i < current_room->n_neighbors; i++)
			if (current_room->neighbors[i]->loaded)
			if (!is_neighbor(current_room->neighbors[i]))
				current_room->neighbors[i]->unload();
		}
		current_room = this;
		if (!loaded) load();
		for (uint i=0; i < n_neighbors; i++)
		if (!neighbors[i]->loaded)
			neighbors[i]->load();
	}

#define ROOM_BEGIN
#define ROOM_POS(...) const Vec pos = {__VA_ARGS__};
#define ROOM_WIDTH(...) const uint16 width = __VA_ARGS__;
#define ROOM_HEIGHT(...) const uint16 height = __VA_ARGS__;
#define ROOM_TILES(...) const int16 tiles [width * height] = {__VA_ARGS__};
#define ROOM_N_NEIGHBORS(...) const uint32 n_neighbors = __VA_ARGS__;
#define ROOM_NEIGHBORS(...) room::Def*const neighbors [n_neighbors] = {__VA_ARGS__};
#define ROOM_N_WALLS(...) const uint32 n_walls = __VA_ARGS__;
#define ROOM_WALLS(...) Wall walls [n_walls] = {__VA_ARGS__};
#define ROOM_INIT(...) void init () {__VA_ARGS__};
#define ROOM_END const room::Def def = {pos, width, height, tiles, n_neighbors, neighbors, n_walls, walls, init, false, NULL};



#include "roomlist.c++"


}









#endif

