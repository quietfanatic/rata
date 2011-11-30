
#ifdef HEADER

struct Wall;

#else


float viewl() { return rata->pos.x - 9; }
float viewr() { return rata->pos.x + 9; }
float viewb() { return rata->pos.y - 5.5; }
float viewt() { return rata->pos.y + 7.5; }



struct Wall {
	const Vec center;
	const float r;
	Vec a;
	Vec b;
	
	Wall (Vec center, float r = 0) :
		center(center),
		r(r)
	{ }

	void build_side (const Wall* prev) {
		if (r == 0 && prev->r == 0) {
			a = prev->center;
			b = center;
		}
			 // Triangle:
			 //  O = center
			 //  | = mag(center - prev->center)
			 //  O = prev->center (angle)
			 //  | = prev->radius
			 //  O   (right angle)
			 //  |
		else {
			float anglediff = -acos((prev->r - r) / mag(center - prev->center));
			a = prev->center + polar(prev->r, ang(center - prev->center) + anglediff);;
			b = center + polar(r, ang(center - prev->center) + anglediff);
		}
	}

	Vec uncross_side (Vec p) const {
		if (!across_line(p, Line(a, b)))
			return Vec::undef;
		else {
			Vec uncross = uncross_line(p, Line(a, b));
			//if (uncross.x < viewl()) {
			//	uncross = Vec(viewl(), liney(a, b, viewl()));
			//}
			//else if (uncross.x > viewr()) {
			//	uncross = Vec(viewr(), liney(a, b, viewr()));
			//}
			//if (uncross.y < viewb()) {
			//	uncross = Vec(linex(a, b, viewb()), viewb());
			//}
			//else if (uncross.y > viewt()) {
			//	uncross = Vec(linex(a, b, viewt()), viewt());
			//}
			return uncross;
		}
	}
	Vec uncross_corner (Vec p, const Wall* next) const {
		if (r > 0) {  // convex
			if (across_line(p, Line(b + rotcw(a - b), b))
			 && across_line(p, Line(next->a, next->a + rotccw(next->b - next->a)))) {
				if (mag2(p - center) < r*r) {
					Vec uncross = center + r * norm(p - center);
					return uncross;
				}
				else return Vec::undef;
			}
			else return Vec::undef;
		}  // concave
		else if (across_line(p, Line(b + rotcw(a - b), b))
		 && across_line(p, Line(next->a, next->a + rotccw(next->b - next->a)))) {
			if (mag2(p - center) > r*r) {
				return center - r * norm(p - center);
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
	if (newp.x < viewl()) newp.x = viewl();
	else if (newp.x > viewr()) newp.x = viewr();
	if (newp.y < viewb()) newp.y = viewb();
	else if (newp.y > viewt()) newp.y = viewt();
	for (uint i=0; i < r->n_walls; i++) {
		 // Wall side (is a line)
		Vec uncross = r->walls[i].uncross_side(p);
		if (defined(uncross)
		 && in_rect(
				uncross,
				Rect(MIN(r->walls[i].a.x, r->walls[i].b.x),
				     MIN(r->walls[i].a.y, r->walls[i].b.y),
				     MAX(r->walls[i].a.x, r->walls[i].b.x),
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





