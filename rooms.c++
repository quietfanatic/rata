
#ifdef DEF_ONLY

namespace room {
	struct Room;
	Room* current = NULL;
	bool transition = false;
	Object* tilemap_obj = NULL;
	struct Room {
		float width;
		float height;
		sf::Color bg_color;
		img::Image* bg_image;
		const int16* tiles;
		uint nobjects;
		const obj::Desc* objects;
		obj::Desc* saved_objects;

		void exit ();
		void start ();
		const int16 tile (uint x, uint y);
		void manifest_tilemap ();
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

	void Room::start () {
		if (current) current->exit();
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

	void Room::exit () {
		transition = true;
		for (Object* o = objects_by_depth; o; o = o->next_depth) {
			o->destroy();
		}
	}

	const int16 Room::tile (uint x, uint y) {
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
			fixdef.friction = 0.4;
			fixdef.restitution = 0;
			//fixdef.density = 100.0;
			tilemap_obj->body->CreateFixture(&fixdef);
		}
	}


#include "rooms_list.c++"


}












#endif
