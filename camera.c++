
#ifdef HEADER

struct Attention {
	float priority;
	Rect range;
	Attention () : priority(-1/0.0) { }
	Attention (float priority, Rect range) :
		priority(priority),
		range(range)
	{ }
};

void propose_attention(Attention);

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

 // WALL USAGE

 // Constrain a point to the walls of the room and the current range.
 // First tries the nearest point on each wall; then, if none are in
 // the range, tries all the intersections between the walls and the
 // range.  If no points are found, gives up with (NaN, NaN).
 // If during phase one it is decided that p is not in a wall, p
 // is returned.
Vec constrain (Vec p, const Rect& range) {
	room::Def* room = current_room;
	bool currently_in = true;
	Vec minp = Vec::undef;
	Vec selectedp = Vec::undef;
	for (uint i=0; i < room->n_walls; i++) {
		if (!across_line(p, bound_a(room->sides[i]))) { // Corner
			if (!across_line(p, bound_b(room->sides[(i?i:room->n_walls)-1]))) {
				Vec tryp = uncross_circle(p, room->walls[i]);
				bool in_this = !in_circle(p, room->walls[i]);
				 // Favor violations a little over nonviolations
				 // The !>= is to deal with NANs.
				if (!(mag2(tryp - p)+in_this >= mag2(minp - p)+currently_in)) {
					currently_in = in_this;
					minp = tryp;
					if (in_rect(tryp, range)) {
						//printf("[%u] In range.\n", frame_number);
						selectedp = tryp;
					}
				}
			}
		}
		else { // Side
			if (across_line(p, bound_b(room->sides[i]))) {
				Vec tryp = uncross_line(p, room->sides[i]);
				bool in_this = !across_line(p, room->sides[i]);
				if (!(mag2(tryp - p)+in_this >= mag2(minp - p)+currently_in)) {
					currently_in = in_this;
					minp = tryp;
					if (in_rect(tryp, range)) {
						selectedp = tryp;
					}
				}
			}
		}
	}
	if (currently_in) return p;
	else if (defined(selectedp)) return selectedp;
	else {  // Get closest intersection between range and walls
		for (uint i=0; i < room->n_walls; i++) {
			const Line& side = room->sides[i];
			Rect siderect = uninvert(Rect(side.a, side.b));
			if (defined(range & siderect)) {
				if (range.l >= siderect.l && range.l <= siderect.r) {
					float y = solvey(side, range.l);
					if (y >= range.b && y <= range.t)
					if (!(mag2(Vec(range.l, y) - p) >= mag2(selectedp - p))) {
						selectedp = Vec(range.l, y);
					}
				}
				if (range.b >= siderect.b && range.b >= siderect.t) {
					float x = solvey(side, range.b);
					if (x >= range.l && x <= range.r)
					if (!(mag2(Vec(x, range.b) - p) >= mag2(selectedp - p))) {
						selectedp = Vec(x, range.b);
					}
				}
				if (range.r >= siderect.l && range.r <= siderect.r) {
					float y = solvey(side, range.r);
					if (y >= range.b && y <= range.t)
					if (!(mag2(Vec(range.r, y) - p) >= mag2(selectedp - p))) {
						selectedp = Vec(range.r, y);
					}
				}
				if (range.t >= siderect.b && range.t >= siderect.t) {
					float x = solvey(side, range.t);
					if (x >= range.l && x <= range.r)
					if (!(mag2(Vec(x, range.t) - p) >= mag2(selectedp - p))) {
						selectedp = Vec(x, range.t);
					}
				}
			}
			const Circle& corner = room->walls[i];
			Rect cornerrect = uninvert(Rect(
				corner.c - Vec(corner.r, corner.r), 
				corner.c + Vec(corner.r, corner.r)
			));
			if (defined(range & cornerrect)) {
				Line abound = bound_a(room->sides[i]);
				Line bbound = bound_b(room->sides[(i?i:room->n_walls)]);
				if (range.l >= cornerrect.l && range.l <= cornerrect.r) {
					float y = sqrt(
						corner.r * corner.r - (range.l - corner.c.x) * (range.l - corner.c.x)
					);
					Vec tryp = Vec(range.l, corner.c.y + y);
					if (tryp.y >= range.b && tryp.y <= range.t)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p))) 
						selectedp = tryp;
					tryp.y = corner.c.y - y;
					if (tryp.y >= range.b && tryp.y <= range.t)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p)))
						selectedp = tryp;
				}
				if (range.b >= cornerrect.b && range.b <= cornerrect.t) {
					float x = sqrt(
						corner.r * corner.r - (range.b - corner.c.y) * (range.b - corner.c.y)
					);
					Vec tryp = Vec(corner.c.x + x, range.b);
					if (tryp.x >= range.l && tryp.x <= range.r)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p)))
						selectedp = tryp;
					tryp.x = corner.c.x - x;
					if (tryp.x >= range.l && tryp.x <= range.r)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p)))
						selectedp = tryp;
				}
				if (range.r >= cornerrect.l && range.r <= cornerrect.r) {
					float y = sqrt(
						corner.r * corner.r - (range.r - corner.c.x) * (range.r - corner.c.x)
					);
					Vec tryp = Vec(range.r, corner.c.y + y);
					if (tryp.y >= range.b && tryp.y <= range.t)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p)))
						selectedp = tryp;
					tryp.y = corner.c.y - y;
					if (tryp.y >= range.b && tryp.y <= range.t)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p)))
						selectedp = tryp;
				}
				if (range.t >= cornerrect.b && range.t <= cornerrect.t) {
					float x = sqrt(
						corner.r * corner.r - (range.t - corner.c.y) * (range.t - corner.c.y)
					);
					Vec tryp = Vec(corner.c.x + x, range.t);
					if (tryp.x >= range.l && tryp.x <= range.r)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p)))
						selectedp = tryp;
					tryp.x = corner.c.x - x;
					if (tryp.x >= range.l && tryp.x <= range.r)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p)))
						selectedp = tryp;
				}
			}
		}
		return selectedp;
	}
}


 // ATTENTION

const uint MAX_ATTENTIONS = 8;
Attention attention [MAX_ATTENTIONS];

void reset_attentions () {
	attention[0] = Attention(200000, Rect(rata->cursor_pos() - Vec(9, 6.5), rata->cursor_pos() + Vec(9, 6.5)));
	attention[1] = Attention(100000, Rect(rata->aim_center() - Vec(9, 6.5), rata->aim_center() + Vec(9, 6.5)));
	for (uint i=2; i < MAX_ATTENTIONS; i++)
		attention[i].priority = -1/0.0;
}

void propose_attention (Attention cur) {
	bool shifting = false;
	for (uint i=0; i < MAX_ATTENTIONS; i++)
	if (shifting || cur.priority > attention[i].priority) {
		shifting = true;
		SWAP(attention[i], cur);
	}
}


 // CAMERA CONTROL

void get_focus () {
	Rect range = attention[0].range;
	 // rwc = range without cursor
	Rect rwc = Rect(-1/0.0, -1/0.0, 1/0.0, 1/0.0);
	for (uint i=1; i < MAX_ATTENTIONS; i++) {
		if (attention[i].priority == -1/0.0) break;
		Rect cur_range = range & attention[i].range;
		if (!defined(cur_range)) continue;
		Rect cur_rwc = rwc & attention[i].range;
		Vec cur_focus;
		cur_focus.x =  (attention[0].range.r - cur_rwc.l)
		             / (size(attention[0].range).x + size(cur_rwc).x)
		             * size(cur_rwc).x
		            + cur_rwc.l;
		cur_focus.y =  (attention[0].range.t - cur_rwc.b)
		             / (size(attention[0].range).y + size(cur_rwc).y)
		             * size(cur_rwc).y
		            + cur_rwc.b;
		oldfocus = cur_focus;
		cur_focus = constrain(cur_focus, cur_range);
		if (defined(cur_focus)) {
			range = cur_range;
			rwc = cur_rwc;
			focus = cur_focus;
		}
	}	 
}



#endif





