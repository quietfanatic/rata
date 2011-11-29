
#ifdef HEADER

struct Wall;

#else

struct Wall {
	Vec a;
	Vec b;
	float radius;
	bool convex;
	Vec uncross_side (Vec p) const {
		if (!across_line(p, a, b))
			return Vec::undef;
		return uncross_line(p, a, b);
	}
	Vec corner_center () const {
		if (convex)
			return b;
		return b + radius * norm(rotcw(a - b));
	}
	Vec uncross_corner (Vec p, const Wall* next) const {
		if (convex)
			return Vec::undef;
		 // Test that we're in this corner.
		if (across_line(p, b + rotccw(a - b), b)
		 && across_line(p, next->a, next->a + rotcw(next->b - next->a))) {
			Vec center = corner_center();
			if (mag2(p - center) > radius*radius) {
				return center + radius * norm(p - center);
			}
			else return Vec::undef;
		}
		else return Vec::undef;
	}
};

Vec constrain (Vec p) {
	room::Def* r = current_room;
	float curdist2 = 1/0.0;
	Vec newp = p;
	for (uint i=0; i < r->n_walls; i++) {
		 // Wall side (is a line)
		Vec uncross = r->walls[i].uncross_side(p);
		if (defined(uncross)) {
			//printf("[%d] Focus vs. uncross: %f,%f vs. %f,%f\n",
			//	frame_number, focus.x, focus.y, uncross.x, uncross.y);
			if (in_rect(
					uncross,
					Vec(MIN(r->walls[i].a.x, r->walls[i].b.x),
					    MIN(r->walls[i].a.y, r->walls[i].b.y)),
					Vec(MAX(r->walls[i].a.x, r->walls[i].b.x),
					    MAX(r->walls[i].a.y, r->walls[i].b.y))
				)
			) {
				//printf("[%d] Focus is crossing a wall.\n", frame_number);
				float dist2 = mag2(uncross - rata->pos);
				if (dist2 < curdist2) {
					//printf("[%d] Uncrossed to side %u at %f.\n", frame_number, i, dist2);
					curdist2 = dist2;
					newp = uncross;
				}
			}
		}
		 // Wall corner (is an arc)
		uncross = r->walls[i].uncross_corner(p, &r->walls[(i+1) % r->n_walls]);
		if (defined(uncross)) {
			float dist2 = mag2(uncross - rata->pos);
			if (dist2 < curdist2) {
				//printf("[%d] Uncrossed to corner %u at %f.\n", frame_number, i, dist2);
				curdist2 = dist2;
				newp = uncross;
			}
		}
	}
	return newp;
}

#endif





