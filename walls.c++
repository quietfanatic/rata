
#ifdef HEADER

struct Wall;

#else

struct Wall {
	Vec a;
	Vec b;
	float radius;
	bool convex;
	Vec corner_center () const {
		if (convex)
			return b;
		return b + radius * norm(rotcw(b - a));
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

#endif





