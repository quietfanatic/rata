
#ifdef DEF_ONLY

namespace room {
	struct Room;
	Room* current = NULL;
	int entrance = -1;
	bool transition = false;
	Object* tilemap_obj = NULL;
	struct Room {
		uint16 width;
		uint16 height;
		sf::Color bg_color;
		int bg_index;
		int16* tiles;
		uint32 nobjects;
		obj::Desc* objects;
		obj::Desc* saved_objects;

		void leave ();
		void enter (int entrance = -1);
		int16 tile (uint x, uint y);
		void manifest_tilemap ();
#ifdef MAPEDITOR
		int print_to_file(FILE* F);
#endif
	};

}
typedef room::Room Room;

#else

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
		manifest_tilemap();
		for (uint i = 0; i < nobjects; i++) {
			if (saved_objects[i].id > 0)
				saved_objects[i].manifest();
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
		return tiles[y * (uint)ceil(width) + x];
	}

	void Room::manifest_tilemap () {
		TileEdge edges [(uint)ceil(width)][(uint)ceil(height)][TILE_MAX_VERTEXES];
		for (uint y=0; y < height; y++)
		for (uint x=0; x < width; x++) {
			bool flip = (tile(x, y) < 0);
			Tileinfo& t = tileinfo[flip? -tile(x,y) : tile(x,y)];
			uint nv = t.nvertexes;
			 // Generate edges
			for (uint e=0; e < TILE_MAX_VERTEXES; e++) {
				if (e < nv) {
					uint n1e = e==0   ? nv-1: e-1;
					uint n2e = e==nv-1? 0   : e+1;
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
				for (uint p=0; p < TILE_MAX_VERTEXES; p++)
				if (edges[x-1][y][p].use)
					maybe_merge_edge(&edges[x][y][e], &edges[x-1][y][p]);
				if (y > 0)
				for (uint p=0; p < TILE_MAX_VERTEXES; p++)
				if (edges[x][y-1][p].use)
					maybe_merge_edge(&edges[x][y][e], &edges[x][y-1][p]);
			}
		}
		 // Transform to b2Fixtures
		tilemap_obj = (new obj::Desc(
			obj::tilemap, NULL,
			Vec(0, 0), Vec(0, 0), 0, true
		))->manifest();
		tilemap_obj->make_body(tilemap_obj->desc, false, false);
		b2FixtureDef fixdef;
		fixdef.filter = cf::solid;
		for (uint x=0; x < width; x++)
		for (uint y=0; y < height; y++)
		for (uint e=0; e < TILE_MAX_VERTEXES; e++)
		if (edges[x][y][e].use) {
			bool flip = (tile(x, y) < 0);
			Tileinfo& t = tileinfo[flip? -tile(x,y) : tile(x,y)];
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
		fprintf(F, "\n\nBEGIN_ROOM\n\nBEGIN_ROOM_TILES\n");
		for (uint y=0; y < height; y++) {
			for (uint x=0; x < width; x++) {
				fprintf(F, "% 3d", tile(x, y));
				if (x+1 != width) fprintf(F, ",");
			}
			if (y+1 != height) fprintf(F, ",\n");
		}
		fprintf(F, "\nEND_ROOM_TILES\n\nBEGIN_ROOM_OBJECTS\n");
		for (uint i=0; i < nobjects; i++) {
			fprintf(F, "ROOM_OBJECT(%s, %f, %f, %f, %f, %d, NULL, %u)\n",
				obj::idname[objects[i].id],
				objects[i].x,
				objects[i].y,
				objects[i].xvel,
				objects[i].yvel,
				objects[i].facing,
				objects[i].data2
			);
		}
		return 0 <= fprintf(F, "END_ROOM_OBJECTS\n\nROOM_DEF(%hu, %hu, %hu, %hhu, %hhu, %hhu, %hhu, %d)\n\nEND_ROOM\n\n",
			width, height, nobjects, bg_color.r, bg_color.g, bg_color.b, bg_color.a, bg_index
		);
	}
#endif

#define BEGIN_ROOM
#define BEGIN_ROOM_TILES int16 tiles [] = {
#define END_ROOM_TILES };
#define BEGIN_ROOM_OBJECTS obj::Desc objects [] = {
#define ROOM_OBJECT(id, x, y, xvel, yvel, facing, data, data2) obj::Desc(id, data, Vec(x, y), Vec(xvel, yvel), facing, false, data2, -1),
#define END_ROOM_OBJECTS obj::Desc()};
#define ROOM_DEF(w, h, o, r, g, b, a, bg) Room room = {w, h, sf::Color(r, g, b, a), bg, tiles, o, objects, NULL};
#define END_ROOM



#include "roomlist.c++"


}












#endif
