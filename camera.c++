
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
	n_debug_points = 0;
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
				reg_debug_point(tryp);
				if (!(mag2(tryp - p)+in_this >= mag2(minp - p)+currently_in)) {
					currently_in = in_this;
					minp = tryp;
					if (!in_this && in_rect(tryp, range)) {
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
				reg_debug_point(tryp);
				if (!(mag2(tryp - p)+in_this >= mag2(minp - p)+currently_in)) {
					currently_in = in_this;
					minp = tryp;
					if (!in_this && in_rect(tryp, range)) {
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
			Rect siderect = aabb(side);
			if (defined(range & siderect)) {
				if (range.l >= siderect.l && range.l <= siderect.r) {
					Vec tryp = Vec(range.l, solvey(side, range.l));
					reg_debug_point(tryp);
					if (tryp.y >= range.b && tryp.y <= range.t)
					if (!(mag2(tryp - p) >= mag2(selectedp - p))) {
						selectedp = tryp;
					}
				}
				if (range.b >= siderect.b && range.b <= siderect.t) {
					Vec tryp = Vec(solvex(side, range.b), range.b);
					reg_debug_point(tryp);
					if (tryp.x >= range.l && tryp.x <= range.r)
					if (!(mag2(tryp - p) >= mag2(selectedp - p))) {
						selectedp = tryp;
					}
				}
				if (range.r >= siderect.l && range.r <= siderect.r) {
					Vec tryp = Vec(range.r, solvey(side, range.r));
					reg_debug_point(tryp);
					if (tryp.y >= range.b && tryp.y <= range.t)
					if (!(mag2(tryp - p) >= mag2(selectedp - p))) {
						selectedp = tryp;
					}
				}
				if (range.t >= siderect.b && range.t <= siderect.t) {
					Vec tryp = Vec(solvex(side, range.t), range.t);
					reg_debug_point(tryp);
					if (tryp.x >= range.l && tryp.x <= range.r)
					if (!(mag2(tryp - p) >= mag2(selectedp - p))) {
						selectedp = tryp;
					}
				}
			}
			const Circle& corner = room->walls[i];
			Rect cornerrect = aabb(corner);
			if (defined(range & cornerrect)) {
				Line abound = bound_a(room->sides[i]);
				Line bbound = bound_b(room->sides[(i?i:room->n_walls)-1]);
				if (range.l >= cornerrect.l && range.l <= cornerrect.r) {
					float y = sqrt(
						corner.r * corner.r - (range.l - corner.c.x) * (range.l - corner.c.x)
					);
					Vec tryp = Vec(range.l, corner.c.y + y);
					reg_debug_point(tryp);
					if (tryp.y >= range.b && tryp.y <= range.t)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p))) 
						selectedp = tryp;
					tryp.y = corner.c.y - y;
					reg_debug_point(tryp);
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
					reg_debug_point(tryp);
					if (tryp.x >= range.l && tryp.x <= range.r)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p)))
						selectedp = tryp;
					tryp.x = corner.c.x - x;
					reg_debug_point(tryp);
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
					reg_debug_point(tryp);
					if (tryp.y >= range.b && tryp.y <= range.t)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p)))
						selectedp = tryp;
					tryp.y = corner.c.y - y;
					reg_debug_point(tryp);
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
					reg_debug_point(tryp);
					if (tryp.x >= range.l && tryp.x <= range.r)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound))
					if (!(mag2(tryp - p) >= mag2(selectedp - p)))
						selectedp = tryp;
					tryp.x = corner.c.x - x;
					reg_debug_point(tryp);
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

 // Move the cursor to a position where it is possible to see
 // both it and Rata.  Basically we need to do an area sweep
 // consisting of a ray cast, two aligned line casts, and two
 // aligned ray casts.
void constrain_cursor () {
	room::Def* room = current_room;
	Line los = Line(rata->cursor_pos(), rata->aim_center());
	 // Offset our ray to the appropriate corner of the screen.
	Vec offset = Vec(9, 6.5);
	if (los.a.x > los.b.x) offset.x = -offset.x;
	if (los.a.y > los.b.y) offset.y = -offset.y;
	los.a += offset;
	los.b += offset;
	debug_line = los;
	Rect linerect = aabb(los);
	Vec edge = rata->aim_center() - offset;
	Rect range = uninvert(Rect(los.b, edge));
	float fraction = 1;  // 1 = b, 0 = a
	for (uint i=0; i < room->n_walls; i++) {
		const Line& side = room->sides[i];
		 // Check side
		if (defined(linerect & aabb(side))) {
			 // Ray
			Vec inter = intersect_lines(los, side);
			//printf("[%u] Checking intersection at %f, %f\n", frame_number, inter.x, inter.y);
			if (in_line(inter, side))
			if (in_line(inter, los)) {
				float newf = line_fraction(inter, los);
				if (newf < fraction) {
					//printf("[%u] Hit side with corner.\n", frame_number);
					fraction = newf;
				}
			}
		}
		if (defined(range & aabb(side))) {
			 // Aligned rays
			float y = solvey(side, edge.x);
			if (y >= linerect.b && y <= linerect.t) {
				float newf = (y - los.a.y) / (los.b.y - los.a.y);
				if (newf < fraction)
					fraction = newf;
			}
			float x = solvex(side, edge.y);
			if (x >= linerect.l && x <= linerect.r) {
				float newf = (x - los.a.x) / (los.b.x - los.a.x);
				if (newf < fraction)
					fraction = newf;
			}
		}
		 // Check corner
		const Circle& corner = room->walls[i];
		Rect cornerrect = aabb(corner);
		if (defined(range & cornerrect)) {
			Line abound = bound_a(room->sides[i]);
			Line bbound = bound_b(room->sides[(i?i:room->n_walls)-1]);
			if (corner.r == 0) {
				 // Only do the aligned lines.
				if (in_rect(corner.c, range)) {
					float xf = (corner.c.x - los.a.x) / (los.b.x - los.a.x);
					float yf = (corner.c.y - los.a.y) / (los.b.y - los.a.y);
					float newf = MAX(xf, yf);
					if (newf < fraction)
						fraction = newf;
				}
			}
			else {
				 // Ray
				if (defined(linerect & cornerrect)) {
					Line inter = intersect_line_circle(los, corner);
					if (defined(inter.a)) {
						float newf = line_fraction(inter.a, los);
						//printf("[%u] inter.a newf = %f\n", frame_number, newf);
						if (newf >= 0 && newf <= 1)
						if (newf < fraction)
						if (!across_line(inter.a, abound))
						if (!across_line(inter.a, bbound)) {
							fraction = newf;
						}
						newf = line_fraction(inter.b, los);
						//printf("[%u] inter.b newf = %f\n", frame_number, newf);
						if (newf >= 0 && newf <= 1)
						if (newf < fraction)
						if (!across_line(inter.b, abound))
						if (!across_line(inter.b, bbound)) {
							fraction = newf;
						}
					}
					else {
						//printf("[%u] No intersections found.\n", frame_number);
					}
				}
				 // Aligned lines
				Vec tryp = Vec(
					corner.c.x + sign_f(los.a.x - los.b.x)*abs_f(corner.r),
					corner.c.y
				);
				if (in_rect(tryp, range)) {
					float xf = (tryp.x - los.a.x) / (los.b.x - los.a.x);
					float yf = (tryp.y - los.a.y) / (los.b.y - los.a.y);
					float newf = MAX(xf, yf);
					if (newf >= 0 && newf <= 1)
					if (newf < fraction)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound)) {
						//printf("[%u] Hit aligned X line at %f, %f.\n", frame_number, tryp.x, tryp.y);
						fraction = newf;
					}
				}
				tryp = Vec(
					corner.c.x,
					corner.c.y + sign_f(los.a.y - los.b.y)*abs_f(corner.r)
				);
				if (in_rect(tryp, range)) {
					float xf = (tryp.x - los.a.x) / (los.b.x - los.a.x);
					float yf = (tryp.y - los.a.y) / (los.b.y - los.a.y);
					float newf = MAX(xf, yf);
					if (newf >= 0 && newf <= 1)
					if (newf < fraction)
					if (!across_line(tryp, abound))
					if (!across_line(tryp, bbound)) {
						//printf("[%u] Hit aligned Y line at %f, %f.\n", frame_number, tryp.x, tryp.y);
						fraction = newf;
					}
				}
				 // Aligned rays
				if (edge.x >= cornerrect.l && edge.x <= cornerrect.r) {
					float y = sqrt(
						corner.r * corner.r - (edge.x - corner.c.x) * (edge.x - corner.c.x)
					);
					Vec tryp = Vec(edge.x, corner.c.y + y);
					if (tryp.y >= linerect.b && tryp.y <= linerect.t) {
						float newf = (tryp.y - los.a.y) / (los.b.y - los.a.y);
						if (newf < fraction)
						if (!across_line(tryp, abound))
						if (!across_line(tryp, bbound))
							fraction = newf;
					}
					tryp.y = corner.c.y - y;
					if (tryp.y >= linerect.b && tryp.y <= linerect.t) {
						float newf = (tryp.y - los.a.y) / (los.b.y - los.a.y);
						if (newf < fraction)
						if (!across_line(tryp, abound))
						if (!across_line(tryp, bbound))
							fraction = newf;
					}
				}
				if (edge.y >= cornerrect.b && edge.y <= cornerrect.t) {
					float x = sqrt(
						corner.r * corner.r - (edge.y - corner.c.y) * (edge.y - corner.c.y)
					);
					Vec tryp = Vec(corner.c.x + x, edge.y);
					if (tryp.x >= linerect.l && tryp.x <= linerect.r) {
						float newf = (tryp.x - los.a.x) / (los.b.x - los.a.x);
						if (newf < fraction)
						if (!across_line(tryp, abound))
						if (!across_line(tryp, bbound))
							fraction = newf;
					}
					tryp.x = corner.c.x - x;
					if (tryp.x >= linerect.l && tryp.x <= linerect.r) {
						float newf = (tryp.x - los.a.x) / (los.b.x - los.a.x);
						if (newf < fraction)
						if (!across_line(tryp, abound))
						if (!across_line(tryp, bbound))
							fraction = newf;
					}
				}
			}
		}
	}
	 // Finally move the cursor back.
	cursor *= 1-fraction;
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
	 // Preliminary cursor restriction
	if (cursor.x < -18)
		cursor = Vec(-18, -18 * slope(cursor));
	else if (cursor.x > 18)
		cursor = Vec(18, 18 * slope(cursor));
	if (cursor.y < -13)
		cursor = Vec(-13 / slope(cursor), -13);
	else if (cursor.y > 13)
		cursor = Vec(13 / slope(cursor), 13);
	attention[0].range = Rect(rata->cursor_pos() - Vec(9, 6.5), rata->cursor_pos() + Vec(9, 6.5));

	Rect range = attention[0].range;
	 // rwc = range without cursor
	Rect rwc = Rect(-1/0.0, -1/0.0, 1/0.0, 1/0.0);
	Vec cur_focus;
	
	bool moved_cursor = false;
	for (uint i=1; i < MAX_ATTENTIONS; i++) {
		if (attention[i].priority == -1/0.0) break;
		Rect cur_rwc = rwc & attention[i].range;
		tryagain:
		Rect cur_range = range & attention[i].range;
		if (!defined(cur_range)) continue;
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
		else if (i == 1 && !moved_cursor) {
			constrain_cursor();
			range = attention[0].range = Rect(rata->cursor_pos() - Vec(9, 6.5), rata->cursor_pos() + Vec(9, 6.5));
			moved_cursor = true;
			goto tryagain;
		}
	}	 
}



#endif





