
#ifdef DEF_ONLY

namespace room {
	struct Room;
	Room* current = NULL;
	Object* tilemap_obj = NULL;
	struct Room {
		float width;
		float height;
		sf::Color bg_color;
		img::Image* bg_image;
		const int16* tiles;
		uint nobjects;
		const obj::Desc* objects;

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
		current = this;
		manifest_tilemap();
		for (uint i = 0; i < nobjects; i++) {
			objects[i].manifest();
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


	const int16 test_t [] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};
	obj::Desc test_o [15] = {
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(20, 1), cf::solid, 0.4)), 10, 0.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 15), cf::solid, 0.4)), 0.5, 7.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 15), cf::solid, 0.4)), 19.5, 7.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(20, 1), cf::solid, 0.4)), 10, 14.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(4, 1), cf::solid, 0.4)), 3, 6.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(5, 1), cf::solid, 0.4)), 8.5, 9.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(4, 1), cf::solid, 0.4)), 8, 3.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 4), cf::solid, 0.4)), 10.5, 12),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 5), cf::solid, 0.4)), 13.5, 3.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(2, 1), cf::solid, 0.4)), 18, 4.5),
		obj::Desc(obj::rata, NULL, 1.5, 3, 0, 0, 1),
		obj::Desc(obj::rat, NULL, 16, 10),
		obj::Desc(obj::rat, NULL, 12, 10),
		obj::Desc(obj::rat, NULL, 7.5, 12),
		obj::Desc(obj::crate, NULL, 8.5, 10.5)
	};
	Room test = {20, 15, sf::Color(127, 127, 127), NULL, test_t, 15, test_o};

	const int16 test2_t [] = {
	//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   // 29
		1, 0,-4,-2, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 2, 4, 0, 0, 0, 1,   // 28
		1,-4,-3, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 3, 4, 0, 0, 1,   // 27
		1,-3, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 3, 4, 0, 1,   // 26
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 4, 1,   // 25
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 3, 1,   // 24
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 1,   // 23
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 1,   // 22
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 1,   // 21
		1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1,   // 20
		1,-2, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 0, 0, 0, 0, 0, 0, 1,   // 19
		1, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4, 0, 0, 0, 0, 0, 1,   // 18
		1, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4, 0, 0, 0, 0, 1,   // 17
		1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,   // 16
		1, 0, 0, 0, 0,-4,-2, 0, 5, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-4,-2, 0, 0, 0, 2, 1,   // 15
		1, 0, 0, 0,-4,-3, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,-4,-3, 0, 0, 0, 0, 0, 1,   // 14
		1, 0, 0,-4,-3, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 1,-2, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0,-4,-3, 0, 0, 0, 0, 0, 0, 1,   // 13
		1, 1, 1, 1, 1, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,-4,-3, 0, 0, 0, 0, 0, 0, 0, 1,   // 12
		1,-4,-2, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1,   // 11
		1,-3, 0, 0, 0, 0, 0, 0, 1, 4, 0, 0, 6, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 6, 0, 0, 0, 0, 1, 1, 0, 0, 5, 0, 0, 5, 0, 0, 0, 1,   // 10
		1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4, 0, 6, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 5, 0, 0, 6, 0, 0, 6, 0, 0, 0, 1,   // 9
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4, 6, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 6, 0, 0, 6, 0, 0, 6, 0, 0, 0, 1,   // 8
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 2, 1, 1,-2, 0, 0, 0, 0, 0, 0, 6, 0, 0, 6, 0, 0, 6, 0, 1, 1, 1,   // 7
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-4,-2, 0, 0, 5, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 1, 0, 0, 6, 0, 0, 2, 1,   // 6
		1, 0, 0, 0, 0, 0, 0, 0, 0,-4,-3, 0, 0, 0, 6, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 1, 0, 0, 6, 0, 0, 0, 1,   // 5
		1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 6, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 1, 0, 0, 6, 0, 0, 0, 1,   // 4
		1, 0, 0, 0, 0, 0, 5, 0, 0, 5, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1,   // 3
		1, 0, 0, 0, 0, 0, 6, 0, 0, 6, 0, 0, 0, 0, 6, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1,   // 2
		1, 0, 0, 0, 0, 0, 6, 0, 0, 6, 0, 0, 0, 0, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 5, 0, 0, 5, 0, 0, 5, 0, 0, 0, 1,   // 1
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1   // 0
	//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39
	};
	// 17,9; 17,10; 25,10; 25,9; 24,9; 24,8; 22,8; 22,5; 20,5; 20,8; 18,8; 18,9; <loop>
	b2Vec2 v0(17, 9);
	b2Vec2 v1(17, 10);
	b2Vec2 v2(25, 10);
	b2Vec2 v3(25, 9);
	b2Vec2 v4(24, 9);
	b2Vec2 v5(24, 8);
	b2Vec2 v6(22, 8);
	b2Vec2 v7(22, 5);
	b2Vec2 v8(20, 5);
	b2Vec2 v9(20, 8);
	b2Vec2 v10(18, 8);
	b2Vec2 v11(18, 9);

	obj::Desc test2_o [24] = {
		/*
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v0, v1, v2, v3), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v1, v2, v3, v4), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v2, v3, v4, v5), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v3, v4, v5, v6), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v4, v5, v6, v7), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v5, v6, v7, v8), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v6, v7, v8, v9), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v7, v8, v9, v10), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v8, v9, v10, v11), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v9, v10, v11, v0), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v10, v11, v0, v1), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_edge(v11, v0, v1, v2), cf::solid, 0.4))),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(40, 1), cf::solid, 0.4)), 20, 0.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(40, 1), cf::solid, 0.4)), 20, 29.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 30), cf::solid, 0.4)), 0.5, 15),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 30), cf::solid, 0.4)), 39.5, 15),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(2, 1), cf::solid, 0.4)), 2, 20.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(13, 1), cf::solid, 0.4)), 13.5, 20.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(12, 1), cf::solid, 0.4)), 27, 20.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(9, 1), cf::solid, 0.4)), 9.5, 16.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(7, 1), cf::solid, 0.4)), 35.5, 16.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 1), cf::solid, 0.4)), 32.5, 15.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(6, 1), cf::solid, 0.4)), 21, 14.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 2), cf::solid, 0.4)), 18.5, 13),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 2), cf::solid, 0.4)), 23.5, 13),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(4, 1), cf::solid, 0.4)), 3, 12.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(8, 1), cf::solid, 0.4)), 33, 11.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 1), cf::solid, 0.4)), 8.5, 10.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(2, 1), cf::solid, 0.4)), 29, 10.5),
		//obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 1), cf::solid, 0.4)), 14.5, 9.5),
		//obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(8, 1), cf::solid, 0.4)), 21, 9.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(2, 1), cf::solid, 0.4)), 28, 9.5),
		//obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(2, 1), cf::solid, 0.4)), 14, 8.5),
		//obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(6, 1), cf::solid, 0.4)), 21, 8.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(4, 1), cf::solid, 0.4)), 13, 7.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(2, 1), cf::solid, 0.4)), 38, 7.5),
		//obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(2, 3), cf::solid, 0.4)), 21, 6.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(6, 1), cf::solid, 0.4)), 8, 4.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 5), cf::solid, 0.4)), 32.5, 4.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 2), cf::solid, 0.4)), 35.5, 3),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(1, 2), cf::solid, 0.4)), 29.5, 3),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(4, 1), cf::solid, 0.4)), 21, 2.5),
		obj::Desc(obj::solid, new b2FixtureDef(make_fixdef(make_rect(10, 1), cf::solid, 0.4)), 20, 1.5),
		*/
		obj::Desc(obj::rata, NULL, 2.5, 22, 0, 0, 1),
		obj::Desc(obj::rat, NULL, 13, 21.5),
		obj::Desc(obj::rat, NULL, 17, 21.5),
		obj::Desc(obj::rat, NULL, 21, 21.5),
		obj::Desc(obj::rat, NULL, 25, 21.5),
		obj::Desc(obj::rat, NULL, 29, 21.5),
		obj::Desc(obj::rat, NULL, 32, 21.5),
		//obj::Desc(obj::crate, NULL, 31.5, 12.5),
		obj::Desc(obj::crate, NULL, 10.5, 17.5),
		obj::Desc(obj::crate, NULL, 6.5, 5.5),
		obj::Desc(obj::crate, NULL, 7.5, 5.5),
		obj::Desc(obj::crate, NULL, 7, 6.5),
		obj::Desc(obj::crate, NULL, 21.5, 15.5),
		obj::Desc(obj::crate, NULL, 22.5, 15.5),
		obj::Desc(obj::crate, NULL, 22, 16.5),
		obj::Desc(obj::crate, NULL, 20.5, 3.5),
		obj::Desc(obj::crate, NULL, 21.5, 4.5),
		obj::Desc(obj::crate, NULL, 21.5, 3.5),
		obj::Desc(obj::crate, NULL, 20.5, 4.5),
		obj::Desc(obj::crate, NULL, 1.5, 1.5),
		obj::Desc(obj::crate, NULL, 2.5, 1.5),
		obj::Desc(obj::mousehole, NULL, 15.5, 21),
		obj::Desc(obj::mousehole, NULL, 27.5, 21),
		//obj::Desc(obj::mousehole, NULL, 38, 17),
		obj::Desc(obj::patroller, NULL, 29.5, 12.5),
		obj::Desc(obj::patroller, NULL, 11.5, 8.5)
	};
	Room test2 = {40, 30, sf::Color(127, 127, 127), &img::bg_metal, test2_t, 24, test2_o};



}












#endif
