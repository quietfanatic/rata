
#ifdef DEF_ONLY

#ifdef MAPEDITOR
#define roomconst
#else
#define roomconst const
#endif

namespace room {
	struct Room;
	Room* current = NULL;
	int currenti = -1;
	int entrance = -1;
	bool transition = false;
	Object* tilemap_obj = NULL;
	struct Room {
		roomconst uint16 width;
		roomconst uint16 height;
		roomconst Color color;
		roomconst int16 bg;
		roomconst int16 bgm;
		roomconst int16* tiles;
		roomconst uint32 nobjects;
		roomconst obj::Desc* objects;
		obj::Desc* saved_objects;

		void leave ();
		void enter (int entrance = -1);
		int16 tile (uint x, uint y);
		void manifest_geometry ();
#ifdef MAPEDITOR
		int print_to_file(FILE* F);
#endif
	};
	enum Location {
		wherever = -1,
		temp = -2,
		inventory = -3,		
	};
	extern Room* list [];
}
typedef room::Room Room;

void enter_room (int id, int entrance);

#else

 // STATIC GEOMETRY GENERATION

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
	Vec v1;
	Vec v2;
	TileEdge* n2;
};



b2EdgeShape* make_edge (Vec v0, Vec v1, Vec v2, Vec v3) {
	b2EdgeShape* r = new b2EdgeShape;
	r->Set(v1, v2);
	r->m_hasVertex0 = r->m_hasVertex3 = 1;
	r->m_vertex0 = v0; r->m_vertex3 = v3;
	return r;
}


inline bool about_eq (float a, float b) {
	return a < b+0.001 && b < a+0.001;
}
inline bool vec_eq (Vec a, Vec b) {
	return about_eq(a.x, b.x) && about_eq(a.y, b.y);
}
int edge_cmp (TileEdge* a, TileEdge* b) {
	if (vec_eq(a->v1, b->v1) && vec_eq(a->v2, b->v2)) return 1;
	else if (vec_eq(a->v1, b->v2) && vec_eq(a->v2, b->v1)) return -1;
	else return 0;
}

/*  CMP > 0
  n1  v1 v1  n1
-------+ +-------
       | |
     A | | B
       | |
-------+ +-------
  n2  v2 v2  n2
    CMP < 0
  n1  v1 v2  n2
-------+ +-------
       | |
     A | | B
       | |
-------+ +-------
  n2  v2 v1  n1
*/

inline void maybe_merge_edge (TileEdge* a, TileEdge* b) {
	int cmp = edge_cmp(a, b);
	if (cmp > 0) {
		a->n1->n2 = b->n1;
		a->n2->n1 = b->n2;
		b->n1->n2 = a->n1;
		b->n2->n1 = a->n2;
		a->use = b->use = false;
	}
	else if (cmp < 0) {  // This should generally be the case.
		a->n1->n2 = b->n2;
		a->n2->n1 = b->n1;
		b->n1->n2 = a->n2;
		b->n2->n1 = a->n1;
		a->use = b->use = false;
	}
}



void enter_room(int roomi, int entrance) {
	room::currenti = roomi;
	room::list[roomi]->enter(entrance);
}

namespace room {
	void Room::enter (int entrance_) {
		camera_jump = true;
		entrance = entrance_;
		if (current) current->leave();
		current = this;
		if (!saved_objects) {
			saved_objects = (obj::Desc*)malloc(nobjects*sizeof(obj::Desc));
			memcpy((void*)saved_objects, (void*)objects, nobjects*sizeof(obj::Desc));
		}
		manifest_geometry();
		map::clear();
		map::load_room(current);
		for (uint i = 0; i < nobjects; i++)
		if (saved_objects[i].id > 0)
			saved_objects[i].manifest();
		for (uint i=0; i < n_saved_things; i++)
		if (saved_things[i].room == currenti)
		if (saved_things[i].id > 0)
			saved_things[i].manifest();
		if (bgm > -1 && bgm != bgm::current) {
			bgm::current = bgm;
			bgm::music.OpenFromFile(bgm::name[bgm]);
			bgm::music.SetLoop(true);
			bgm::music.SetVolume(80);
			bgm::music.Play();
		}
	}

	void Room::leave () {
		transition = true;
		for (Object* o = objects_by_depth; o; o = o->next_depth) {
			if (o->desc->id != obj::rata) {
				o->destroy();
			}
		}
		for (uint i=0; i < MAX_BULLETS; i++)
			bullets[i].lifetime = -1;
	}

	int16 Room::tile (uint x, uint y) {
		return tiles[y * width + x];
	}

	void Room::manifest_geometry () {
		TileEdge edges [(uint)ceil(width)][(uint)ceil(height)][tile::max_vertexes];
		for (uint y=0; y < height; y++)
		for (uint x=0; x < width; x++) {
			bool flip = (tile(x, y) < 0);
			const tile::Def& t = tile::def[flip? -tile(x,y) : tile(x,y)];
			uint nv = t.nvertexes;
			 // Generate edges
			for (uint e=0; e < tile::max_vertexes; e++) {
				if (e < nv) {
					uint n1e = (nv+e - 1) % nv;
					uint n2e = (nv+e + 1) % nv;
					edges[x][y][e].use = true;
					edges[x][y][e].n1 = &edges[x][y][n1e];
					edges[x][y][e].v1 = t.vertexes[e] + Vec(x, height-y-1);
					edges[x][y][e].v2 = t.vertexes[n2e] + Vec(x, height-y-1);
					edges[x][y][e].n2 = &edges[x][y][n2e];
				}
				else edges[x][y][e].use = false;
			}
			 // Merge edges
			for (uint e=0; e < nv; e++) {
				if (x > 0)
				for (uint p=0; p < tile::max_vertexes; p++)
				if (edges[x-1][y][p].use)
					maybe_merge_edge(&edges[x][y][e], &edges[x-1][y][p]);
				if (y > 0)
				for (uint p=0; p < tile::max_vertexes; p++)
				if (edges[x][y-1][p].use)
					maybe_merge_edge(&edges[x][y][e], &edges[x][y-1][p]);
			}
		}
		 // Transform to b2Fixtures
		tilemap_obj = (new obj::Desc(
			-2, obj::tilemap
		))->manifest();
		tilemap_obj->make_body(tilemap_obj->desc, false, false);
		b2FixtureDef fixdef;
		fixdef.filter = cf::solid;
		for (uint x=0; x < width; x++)
		for (uint y=0; y < height; y++)
		for (uint e=0; e < tile::max_vertexes; e++)
		if (edges[x][y][e].use) {
			bool flip = (tile(x, y) < 0);
			const tile::Def& t = tile::def[flip? -tile(x,y) : tile(x,y)];
			b2EdgeShape* edge = new b2EdgeShape;
			edge->Set(edges[x][y][e].v1, edges[x][y][e].v2);
			if (edges[x][y][e].n1) {
				edge->m_hasVertex0 = true;
				edge->m_vertex0 = edges[x][y][e].n1->v1;
			}
			if (edges[x][y][e].n2) {
				edge->m_hasVertex3 = true;
				edge->m_vertex3 = edges[x][y][e].n2->v2;
			}
			fixdef.shape = edge;
			fixdef.friction = t.friction;
			fixdef.restitution = 0;
			//fixdef.density = 100.0;
			fixdef.userData = t.prop ? t.prop : &default_FixProp;
			tilemap_obj->body->CreateFixture(&fixdef);
		}
	}

	void die (const char* mess) {
		fprintf(stderr, mess);
		exit(1);
	}

#ifdef MAPEDITOR
	int Room::print_to_file (FILE* F) {
		fprintf(F,
			"\n\n"
			"ROOM_BEGIN\n\n"
			"ROOM_WIDTH(%hu)\n"
			"ROOM_HEIGHT(%hu)\n"
			"ROOM_TILES(\n", width, height
		);

		for (uint y=0; y < height; y++) {
			for (uint x=0; x < width; x++) {
				fprintf(F, "% 3d,", tile(x, y));
			}
			fprintf(F, "\n");
		}
		fprintf(F,
			")\n\n"
			"ROOM_NOBJECTS(%hu)\n"
			"ROOM_OBJECTS(\n", nobjects
		);
		for (uint i=0; i < nobjects; i++) {
			fprintf(F, "\tROOM_OBJECT(%s, %f, %f, %f, %f, %d, %u, %u)\n",
				obj::idname[objects[i].id],
				objects[i].pos.x,
				objects[i].pos.y,
				objects[i].vel.x,
				objects[i].vel.y,
				objects[i].facing,
				objects[i].data,
				objects[i].data2
			);
		}
		return 0<=fprintf(F,
			")\n\n"
			"ROOM_COLOR(%hhu, %hhu, %hhu, %hhu)\n"
			"ROOM_BG(%hd)\n"
			"ROOM_BGM(%hd)\n\n"
			"ROOM_END\n\n",
			color.r, color.g, color.b, color.a,
			bg, bgm
		);
	}
#endif

#define ROOM_BEGIN
#define ROOM_WIDTH(...) static const uint16 width = __VA_ARGS__;
#define ROOM_HEIGHT(...) static const uint16 height = __VA_ARGS__;
#define ROOM_TILES(...) roomconst int16 tiles [width * height] = {__VA_ARGS__};
#define ROOM_NOBJECTS(...) static const uint nobjects = __VA_ARGS__;
#define ROOM_OBJECTS(...) roomconst obj::Desc objects [nobjects] = {__VA_ARGS__};
#define ROOM_OBJECT(id, x, y, xvel, yvel, facing, data, data2) obj::Desc(-1, id, Vec(x, y), Vec(xvel, yvel), facing, data, data2),
#define ROOM_COLOR(...) static const Color color = Color(__VA_ARGS__);
#define ROOM_BG(...) static const uint16 bg = __VA_ARGS__;
#define ROOM_BGM(...) static const uint16 bgm = __VA_ARGS__;
#define ROOM_END Room room = {width, height, color, bg, bgm, tiles, nobjects, objects, NULL};



#include "roomlist.c++"


}












#endif
