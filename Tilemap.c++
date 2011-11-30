

#ifdef HEADER

struct Tilemap : Object {
	void after_move () { }
	void before_move () { }
	Tilemap (room::Def* def);
};

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



//b2EdgeShape make_edge (Vec v0, Vec v1, Vec v2, Vec v3) {
//	b2EdgeShape* r = new b2EdgeShape;
//	r->Set(v1, v2);
//	r->m_hasVertex0 = r->m_hasVertex3 = 1;
//	r->m_vertex0 = v0; r->m_vertex3 = v3;
//	return r;
//}


bool about_eq (float a, float b) {
	return a < b+0.001 && b < a+0.001;
}
bool vec_eq (Vec a, Vec b) {
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

void maybe_merge_edge (TileEdge* a, TileEdge* b) {
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





Tilemap::Tilemap (room::Def* def) :
	Object(type::tilemap, def, def->pos)
{
	 // Create geometry
	TileEdge edges [(uint)ceil(def->width)][(uint)ceil(def->height)][tile::max_vertexes];
	for (uint y=0; y < def->height; y++)
	for (uint x=0; x < def->width; x++) {
		bool flip = (def->tile(x, y) < 0);
		const tile::Def& t = tile::def[flip? -def->tile(x,y) : def->tile(x,y)];
		uint nv = t.nvertexes;
		 // Generate edges
		for (uint e=0; e < tile::max_vertexes; e++) {
			if (e < nv) {
				uint n1e = (nv+e - 1) % nv;
				uint n2e = (nv+e + 1) % nv;
				edges[x][y][e].use = true;
				edges[x][y][e].n1 = &edges[x][y][n1e];
				edges[x][y][e].v1 = t.vertexes[e] + Vec(x, def->height-y-1);
				edges[x][y][e].v2 = t.vertexes[n2e] + Vec(x, def->height-y-1);
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
	body = make_body(b2_staticBody, false);
	body->SetActive(true);
	b2FixtureDef fixdef;
	fixdef.filter = cf::solid;
	uint nfixes=0;
	for (uint x=0; x < def->width; x++)
	for (uint y=0; y < def->height; y++)
	for (uint e=0; e < tile::max_vertexes; e++)
	if (edges[x][y][e].use) {
		bool flip = (def->tile(x, y) < 0);
		const tile::Def& t = tile::def[flip? -def->tile(x,y) : def->tile(x,y)];
		b2EdgeShape edge;
		edge.Set(edges[x][y][e].v1, edges[x][y][e].v2);
		if (edges[x][y][e].n1) {
			edge.m_hasVertex0 = true;
			edge.m_vertex0 = edges[x][y][e].n1->v1;
		}
		if (edges[x][y][e].n2) {
			edge.m_hasVertex3 = true;
			edge.m_vertex3 = edges[x][y][e].n2->v2;
		}
		fixdef.shape = &edge;
		fixdef.friction = t.friction;
		fixdef.restitution = 0;
		//fixdef.density = 100.0;
		fixdef.userData = t.prop ? t.prop : &default_FixProp;
		body->CreateFixture(&fixdef);
		nfixes++;
	}
	printf("Created %u fixtures.\n", nfixes);

}



#endif
