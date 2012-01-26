
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

void propose_attention (Attention);

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

 // ATTENTION

const uint MAX_ATTENTIONS = 8;
Attention attention [MAX_ATTENTIONS];

void reset_attentions () {
	attention[0] = Attention(1000000, Rect(rata->aim_center() - Vec(9, 6.5), rata->aim_center() + Vec(9, 6.5)));
	for (uint i=1; i < MAX_ATTENTIONS; i++)
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
						//dbg_camera("[%u] In range.\n");
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
	if (in_rect(p, range) && currently_in) {
		//dbg_camera("Currently in, %f, %f\n", p.x, p.y);
		return p;
	}
	else if (defined(selectedp)) {
		//dbg_camera("Moved to wall, %f, %f -> %f, %f\n", p.x, p.y, selectedp.x, selectedp.y);
		return selectedp;
	}
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
		//dbg_camera("Moved to wall and range, %f, %f -> %f, %f\n", p.x, p.y, selectedp.x, selectedp.y);
		return selectedp;
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
	 // Secondary cursor restriction
	Vec prim_focus = (rata->cursor_pos() + rata->aim_center()) / 2;
	prim_focus = constrain(prim_focus, attention[0].range);
	Vec rel_focus = prim_focus - rata->aim_center();
	if (cursor.x < rel_focus.x - 10)
		cursor = Vec(rel_focus.x - 10, (rel_focus.x - 10) * slope(cursor));
	else if (cursor.x > rel_focus.x + 10)
		cursor = Vec(rel_focus.x + 10, (rel_focus.x + 10) * slope(cursor));
	if (cursor.y < rel_focus.y - 7.5)
		cursor = Vec((rel_focus.y - 7.5) / slope(cursor), rel_focus.y - 7.5);
	else if (cursor.y > rel_focus.y + 7.5)
		cursor = Vec((rel_focus.y + 7.5) / slope(cursor), rel_focus.y + 7.5);
	 // Stretch cursor range
	Vec cursorrange_v;
	if (cursor.x < rel_focus.x - 9)
		cursorrange_v.x = rel_focus.x - cursor.x;
	else if (cursor.x > rel_focus.x + 9)
		cursorrange_v.x = cursor.x - rel_focus.x;
	else cursorrange_v.x = 9;
	if (cursor.y < rel_focus.y - 7.5)
		cursorrange_v.y = rel_focus.y - cursor.y;
	else if (cursor.y > rel_focus.y + 7.5)
		cursorrange_v.y = cursor.y - rel_focus.y;
	else cursorrange_v.y = 7.5;
	Rect cursorrange = Rect(rata->cursor_pos() - cursorrange_v, rata->cursor_pos() + cursorrange_v);
	 // rwc = Range Without Cursor
	Rect rwc = Rect(-1/0.0, -1/0.0, 1/0.0, 1/0.0);
	Rect range = cursorrange;
	

	Vec cur_focus;
	
	bool moved_cursor = false;
	for (uint i=0; i < MAX_ATTENTIONS; i++) {
		if (attention[i].priority == -1/0.0) break;
		Rect cur_rwc = rwc & attention[i].range;
		tryagain:
		Rect cur_range = range & attention[i].range;
		if (!defined(cur_range)) continue;
		cur_focus.x =  (cursorrange.r - cur_rwc.l)
		             / (size(cursorrange).x + size(cur_rwc).x)
		             * size(cur_rwc).x
		            + cur_rwc.l;
		cur_focus.y =  (cursorrange.t - cur_rwc.b)
		             / (size(cursorrange).y + size(cur_rwc).y)
		             * size(cur_rwc).y
		            + cur_rwc.b;
		if (cur_focus.x < cur_range.l) cur_focus.x = cur_range.l;
		if (cur_focus.y < cur_range.b) cur_focus.y = cur_range.b;
		if (cur_focus.x > cur_range.r) cur_focus.x = cur_range.r;
		if (cur_focus.y > cur_range.t) cur_focus.y = cur_range.t;
		oldfocus = cur_focus;
		cur_focus = constrain(cur_focus, cur_range);
		if (defined(cur_focus)) {
			range = cur_range;
			rwc = cur_rwc;
			focus = cur_focus;
		}
	}
}

void get_camera () {
	 // To look smooth in a pixelated environment,
	 //  we need a minimum speed.
	 // We also need to hold camera pixel-steady
	 //  to Rata pos when running.
	float snap_dist = camera_snap ? .5*PX : .25*PX;
	camera_snap = false;
	if (camera_jump) {
		camera = focus;
		camera_jump = false;
	}
	else {
		if (abs_f(focus.x - camera.x) < .25*PX) camera.x = focus.x;
		else {
			float xvel = (focus.x - camera.x) / 4;
			if (abs_f(xvel) < .25*PX)
				camera.x += .25*PX * sign_f(xvel);
			else if (abs_f((xvel) - rata->vel.x/FPS) < snap_dist) {
				camera.x = old_camera_rel + round(rata->pos.x*UNPX)*PX;
				camera_snap = true;
			}
			else
				camera.x += xvel;
		}
		if (abs_f(focus.y - camera.y) < .25*PX) camera.y = focus.y;
		else {
			float yvel = (focus.y - camera.y) / 4;
			if (abs_f(yvel) < .25*PX)
				camera.y += .25*PX * sign_f(yvel);
			else
				camera.y += yvel;
		}
	}
	old_camera_rel = camera.x - round(rata->pos.x*UNPX)*PX;
}



#endif





