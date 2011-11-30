
#ifdef HEADER

struct Wall;

#else


float viewl() { return rata->pos.x - 9; }
float viewr() { return rata->pos.x + 9; }
float viewb() { return rata->pos.y - 5.5; }
float viewt() { return rata->pos.y + 7.5; }


Line get_tangent (const Circle& a, const Circle& b) {
	if (a.r == 0 && b.r == 0) {
		return Line(a.c, b.c);
	}
		 // Triangle:
		 //  O = center
		 //  | = mag(center - prev->center)
		 //  O = prev->center (angle)
		 //  | = prev->radius
		 //  O   (right angle)
		 //  |
	else {
		float angle = ang(b.c - a.c) - acos((a.r - b.r) / mag(b.c - a.c));
		return Line(
			a.c + polar(a.r, angle),
			b.c + polar(b.r, angle)
		);
	}
}

void build_sides (const room::Def* r) {
	for (uint i=0; i < r->n_walls; i++) {
		r->sides[i] = get_tangent(r->walls[i], r->walls[(i+1) % r->n_walls]);
	}
}



Vec constrain (Vec p) {
	room::Def* r = current_room;
	float curdist2 = 1/0.0;
	Vec newp = p;
	if (newp.x < viewl()) newp.x = viewl();
	else if (newp.x > viewr()) newp.x = viewr();
	if (newp.y < viewb()) newp.y = viewb();
	else if (newp.y > viewt()) newp.y = viewt();
//	for (uint i=0; i < r->n_walls; i++) {
//		
//	}
	return newp;
}

#endif





