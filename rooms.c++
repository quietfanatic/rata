
#ifdef DEF_ONLY

namespace room {
	struct Room;
	Room* current = NULL;
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
		void enter ();
		int16 tile (uint x, uint y);
		void manifest_tilemap ();
		void read_from_file(FILE* F);
	};

}
typedef room::Room Room;

#else

b2EdgeShape* make_edge (b2Vec2 v0, b2Vec2 v1, b2Vec2 v2, b2Vec2 v3) {
	b2EdgeShape* r = new b2EdgeShape;
	r->Set(v1, v2);
	r->m_hasVertex0 = r->m_hasVertex3 = 1;
	r->m_vertex0 = v0; r->m_vertex3 = v3;
	return r;
}


inline bool about_eq (float a, float b) {
	return a < b+0.001 && b < a+0.001;
}
inline bool vec_eq (b2Vec2 a, b2Vec2 b) {
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

	void Room::enter () {
		if (current) current->leave();
		current = this;
		if (!saved_objects) {
			saved_objects = (obj::Desc*)malloc(nobjects*sizeof(obj::Desc));
			memcpy((void*)saved_objects, (void*)objects, nobjects*sizeof(obj::Desc));
		}
		manifest_tilemap();
		for (uint i = 0; i < nobjects; i++) {
			saved_objects[i].manifest();
		}
	}

	void Room::leave () {
		transition = true;
		for (Object* o = objects_by_depth; o; o = o->next_depth) {
			o->destroy();
		}
	}

	int16 Room::tile (uint x, uint y) {
		return tiles[y * (uint)ceil(width) + x];
	}

	void Room::manifest_tilemap () {
		TileEdge edges [(uint)ceil(width)][(uint)ceil(height)][4];
		for (uint y=0; y < height; y++)
		for (uint x=0; x < width; x++) {
			bool flip = (tile(x, y) < 0);
			Tileinfo& t = tileinfo[flip? -tile(x,y) : tile(x,y)];
			uint nv = t.nvertexes;
			 // Generate edges
			for (uint e=0; e < 4; e++) {
				if (e < nv) {
					uint n1e = e==0   ? nv-1: e-1;
					uint n2e = e==nv-1? 0   : e+1;
					edges[x][y][e] = {
						true,
						&edges[x][y][n1e],
						t.vertexes[e] + b2Vec2(x, height-y-1),
						t.vertexes[n2e] + b2Vec2(x, height-y-1),
						&edges[x][y][n2e]
					};
				}
				else edges[x][y][e].use = false;
			}
			 // Merge edges
			for (uint e=0; e < nv; e++) {
				if (x > 0)
				for (uint p=0; p < 4; p++)
				if (edges[x-1][y][p].use)
					maybe_merge_edge(&edges[x][y][e], &edges[x-1][y][p]);
				if (y > 0)
				for (uint p=0; p < 4; p++)
				if (edges[x][y-1][p].use)
					maybe_merge_edge(&edges[x][y][e], &edges[x][y-1][p]);
			}
		}
		 // Transform to b2Fixtures
		tilemap_obj = (new obj::Desc(
			obj::tilemap, NULL,
			0, 0, 0, 0, 0, true
		))->manifest();
		tilemap_obj->make_body(tilemap_obj->desc, false, false);
		b2FixtureDef fixdef;
		fixdef.filter = cf::solid;
		for (uint x=0; x < width; x++)
		for (uint y=0; y < height; y++)
		for (uint e=0; e < 4; e++)
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
			tilemap_obj->body->CreateFixture(&fixdef);
		}
	}

	void die (const char* mess) {
		fprintf(stderr, mess);
		exit(1);
	}

#define GB bytecount++; if (1 != fread(&b, 1, 1, F)) { fprintf(stderr, "Room load error: ran out of bytes at %d.\n", bytecount); exit(1); }

	void Room::read_from_file (FILE* F) {
		tiles = new int16 [(int)(width*height)];
		objects = new obj::Desc [nobjects];
		int bytecount = -1;
		uint8 b;
		GB  width = b<<8;
		GB  width |= b;
		GB  height = b<<8;
		GB  height |= b;
		printf("Room is %d x %d\n", width, height);
		for (int i=0; i < width*height; i++) {
			GB  tiles[i] = b<<8;
			GB  tiles[i] |= b;
		}
		GB  nobjects = b<<24;
		GB  nobjects |= b<<16;
		GB  nobjects |= b<<8;
		GB  nobjects |= b;
		for (int i=0; i < nobjects; i++) {
			GB  objects[i].id = b<<8;
			GB  objects[i].id |= b;
			GB  GB  objects[i].temp = 0;
			GB  *(uint*)&objects[i].x = b<<24;
			GB  *(uint*)&objects[i].x |= b<<16;
			GB  *(uint*)&objects[i].x |= b<<8;
			GB  *(uint*)&objects[i].x |= b;
			GB  *(uint*)&objects[i].y = b<<24;
			GB  *(uint*)&objects[i].y |= b<<16;
			GB  *(uint*)&objects[i].y |= b<<8;
			GB  *(uint*)&objects[i].y |= b;
			GB  *(uint*)&objects[i].xvel = b<<24;
			GB  *(uint*)&objects[i].xvel |= b<<16;
			GB  *(uint*)&objects[i].xvel |= b<<8;
			GB  *(uint*)&objects[i].xvel |= b;
			GB  *(uint*)&objects[i].yvel = b<<24;
			GB  *(uint*)&objects[i].yvel |= b<<16;
			GB  *(uint*)&objects[i].yvel |= b<<8;
			GB  *(uint*)&objects[i].yvel |= b;
			GB  *(uint*)&objects[i].facing = b<<24;
			GB  *(uint*)&objects[i].facing |= b<<16;
			GB  *(uint*)&objects[i].facing |= b<<8;
			GB  *(uint*)&objects[i].facing |= b;
			GB  *(uint*)&objects[i].data = b<<24;
			GB  *(uint*)&objects[i].data |= b<<16;
			GB  *(uint*)&objects[i].data |= b<<8;
			GB  *(uint*)&objects[i].data |= b;
			GB  objects[i].data2 = b<<24;
			GB  objects[i].data2 |= b<<16;
			GB  objects[i].data2 |= b<<8;
			GB  objects[i].data2 |= b;
		}
		GB  bg_color.r = b;
		GB  bg_color.g = b;
		GB  bg_color.b = b;
		GB  bg_color.a = b;
		GB  *(uint*)&bg_index = b<<24;
		GB  *(uint*)&bg_index |= b<<16;
		GB  *(uint*)&bg_index |= b<<8;
		GB  *(uint*)&bg_index |= b;
		saved_objects = NULL;
	}


		/*
		if (1 != fread(&width, 2, 1, F)) die("Error: ran out of binary input at width.\n");
		width = (width/256) + 256*(width%256);
		if (1 != fread(&height, 2, 1, F)) die("Error: ran out of binary input at height.\n");
		height = (height/256) + 256*(height%256);
		tiles = new int16 [(int)(width*height)];
		for (int i=0; i < width*height; i++) {  // Swap byte order
			int16* ts = (int16*)tiles;  // Unconst
			ts[i] = (ts[i]/256) + 256*(ts[i]%256);
		}
		if (1 != fread((int16*)tiles, width*height*2, 1, F)) die("Error: ran out of binary input in tiles.\n");
		if (1 != fread(&nobjects, 2, 1, F)) die("Error: ran out of binary input at nobjects.\n");
		objects = new obj::Desc [nobjects];
		if (1 != fread((obj::Desc*)objects, nobjects*32, 1, F)) die("Error: ran out of binary input in objects.\n");
		if (1 != fread(&bg_color, 4, 1, F)) die("Error: ran out of binary input at bg_color.\n");
		if (1 != fread(&bg_index, 4, 1, F)) die("Error: ran out of binary input at bg_index.\n");
		saved_objects = NULL;
	}*/



#include "roomlist.c++"


}












#endif
