
#ifdef HEADER

struct Wall;

#else

 // WALL CONSTRUCTION

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

 // WALL GEOMETRY

Vec constrain (Vec p) {
	room::Def* r = current_room;
	float curdist2 = 1/0.0;
	Vec newp = p;
//	for (uint i=0; i < r->n_walls; i++) {
//		
//	}
	return newp;
}


 // ATTENTION

struct Attention {
	float priority;
	Rect range;
	Attention () : priority(-1/0.0) { }
	Attention (float priority, Rect range) :
		priority(priority),
		range(range)
	{ }
};

const uint MAX_ATTENTIONS = 8;
Attention attention [MAX_ATTENTIONS];

void reset_attentions () {
	attention[0] = Attention(200000, Rect(rata->cursor_pos() - Vec(9, 6.5), rata->cursor_pos() + Vec(9, 6.5)));
	attention[1] = Attention(100000, Rect(rata->aim_center() - Vec(9, 6.5), rata->aim_center() + Vec(9, 6.5)));
	for (uint i=2; i < MAX_ATTENTIONS; i++)
		attention[i].priority = -1/0.0;
}

void propose_attention (Attention cur) {
	bool shifting;
	for (uint i=0; i < MAX_ATTENTIONS; i++)
	if (shifting || cur.priority > attention[i].priority)
		SWAP(attention[i], cur);
}


 // CAMERA CONTROL

void get_focus () {
	Rect range = attention[0].range & attention[1].range;
	Rect range_but_cursor = attention[1].range;
	for (uint i=2; i < MAX_ATTENTIONS; i++) {
		if (attention[i].priority == -1/0.0) break;
		Rect cur_range = range & attention[i].range;
		if (defined(cur_range)) {
			 // todo: ensure range is within walls
			range = cur_range;
			range_but_cursor &= attention[i].range;
		}
	}
	focus.x = (attention[0].range.r - range_but_cursor.l)
	        / (size(attention[0].range).x + size(range_but_cursor).x)
			* size(range_but_cursor).x
			+ range_but_cursor.l;
	focus.y = (attention[0].range.t - range_but_cursor.b)
	        / (size(attention[0].range).y + size(range_but_cursor).y)
			* size(range_but_cursor).y
			+ range_but_cursor.b;
		 
}



#endif





