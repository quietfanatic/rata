
#ifdef HEADER

struct Wall;

#else


float viewl() { return rata->pos.x - 9; }
float viewr() { return rata->pos.x + 9; }
float viewb() { return rata->pos.y - 5.5; }
float viewt() { return rata->pos.y + 7.5; }



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
			 // Triangle:
			 //  O = center
			 //  | = mag(center - prev->center)
			 //  O = prev->center (angle)
			 //  | = prev->radius
			 //  O   (right angle)
			 //  |
		else {
			float pr = prev->convex ? -prev->radius : prev->radius;
			float r = convex ? -radius : radius;
			float anglediff = acos((pr - r) / mag(center - prev->center));
			a = prev->center + polar(pr, ang(center - prev->center) + anglediff);;
			b = center + polar(r, ang(center - prev->center) + anglediff);
		}
	}

	Vec uncross_side (Vec p) const {
		if (!across_line(p, a, b))
			return Vec::undef;
		else {
			Vec uncross = uncross_line(p, a, b);
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
		if (convex) {
			if (across_line(p, b + rotccw(a - b), b)
			 && across_line(p, next->a, next->a + rotcw(next->b - next->a))) {
				if (mag2(p - center) < radius*radius) {
					Vec uncross = center + radius * norm(p - center);
					if (uncross.x < viewl()) {
						float sign = uncross.y < center.y ? -1 : 1;
						uncross = Vec(viewl(), center.y + sign*sqrt(radius*radius - (center.x-viewl())*(center.x-viewl())));
					}
					else if (uncross.x > viewr()) {
						float sign = uncross.y < center.y ? -1 : 1;
						uncross = Vec(viewr(), center.y + sign*sqrt(radius*radius - (viewr()-center.x)*(viewr()-center.x)));
					}
					if (uncross.y < viewb()) {
						float sign = uncross.x < center.x ? -1 : 1;
						uncross = Vec(center.x + sign*sqrt(radius*radius - (center.y-viewb())*(center.y-viewb())), viewb());
					}
					else if (uncross.y > viewt()) {
						float sign = uncross.x < center.x ? -1 : 1;
						uncross = Vec(center.x + sign*sqrt(radius*radius - (viewt()-center.y)*(viewt()-center.y)), viewt());
					}
					return uncross;
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





