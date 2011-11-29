
#ifdef HEADER

struct Wall;

#else

struct Wall {
	const Vec center;
	const float radius;
	const bool convex;
	Vec a;
	Vec b;
	
	Wall (Vec center, float radius = 0, bool convex = false) :
		center(center),
		radius(radius),
		convex(convex)
	{ }

	void build_side (const Wall* prev) {
		if (radius == 0 && prev->radius == 0) {
			a = prev->center;
			b = center;
		}
		else {
			a = prev->center;
			b = center;
		}
	}

	Vec uncross_side (Vec p) const {
		if (!across_line(p, a, b))
			return Vec::undef;
		else
			return uncross_line(p, a, b);
	}
	Vec uncross_corner (Vec p, const Wall* next) const {
		if (convex) {
			 // We only need to check the later angle
			 // because the earlier one was checked when
			 // testing the side.
			if (across_line(p, next->a, next->a + rotcw(next->b - next->a))) {
				if (mag2(p - center) < radius*radius) {
					return center + radius * norm(p - center);
				}
				else return Vec::undef;
			}
			else return Vec::undef;
		}  // concave
		else if (across_line(p, b + rotccw(a - b), b)
		 && across_line(p, next->a, next->a + rotcw(next->b - next->a))) {
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
		if (defined(uncross)
		 && in_rect(
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
		else {
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
	}
	return newp;
}

#endif





