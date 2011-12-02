

 // GENERIC
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SWAP(a, b) {typeof(a) _t = b; b = a; a = _t;}
 // FLOAT
float mod_f (float a, float b) {
	return a >= 0 ? a - b*(double)(int)(a/b)
	              : a - b*(double)(int)(a/b);
}
float abs_f (float x) { return x>0 ? x : -x; }
int sign_f (float x) { return (x>0)-(x<0); }

float dither (float c, float err) {
	return c - err + rand()*2.0*err/RAND_MAX;
}

 // ANGLES
float flip_angle (float a) {
	return a<0 ? -pi-a : pi-a;
}
bool gt_angle (float a, float b) {
	if (b < 0)
		return a > b && a < b+pi;
	else return a > b || a < b-pi;
}
 // Using angles in animation
uint8 get_angle_frame (float a) {
	if (a != a) return 4;
	if (gt_angle(a, pi/2)) {
		a = flip_angle(a);
	}
	return
	  a > pi * +7/16.0 ? 8
	: a > pi * +5/16.0 ? 7
	: a > pi * +3/16.0 ? 6
	: a > pi * +1/16.0 ? 5
	: a > pi * -1/16.0 ? 4
	: a > pi * -3/16.0 ? 3
	: a > pi * -5/16.0 ? 2
	: a > pi * -7/16.0 ? 1
	:                    0;
}

 // VECTORS

 // Our wheels are better.
struct Vec {
	float x;
	float y;
	Vec () { }
	Vec (float x, float y) :x(x), y(y) { }
	Vec (b2Vec2 v) :x(v.x), y(v.y) { }
	Vec (sf::Vector2<float> v) :x(v.x), y(v.y) { }
	operator const b2Vec2 () const { return b2Vec2(x, y); }
	operator const sf::Vector2<float> () const { return sf::Vector2<float>(x, y); }

	Vec scalex (float s) const { return Vec(s*x, y); }
	Vec scaley (float s) const { return Vec(x, s*y); }

	static const Vec undef;
};
const Vec Vec::undef = Vec(0/0.0, 0/0.0);


Vec operator - (Vec a) { return Vec(-a.x, -a.y); }
Vec operator + (Vec a, Vec b) { return Vec(a.x+b.x, a.y+b.y); }
Vec operator - (Vec a, Vec b) { return Vec(a.x-b.x, a.y-b.y); }
Vec operator * (Vec a, float b) { return Vec(a.x*b, a.y*b); }
Vec operator * (float a, Vec b) { return Vec(a*b.x, a*b.y); }
Vec operator / (Vec a,  float b) { return Vec(a.x/b, a.y/b); }
Vec operator += (Vec& a, Vec b) { return a = a + b; }
Vec operator -= (Vec& a, Vec b) { return a = a - b; }
Vec operator *= (Vec& a, float b) { return a = a * b; }
Vec operator /= (Vec& a, float b) { return a = a / b; }
bool operator == (Vec a, Vec b) { return a.x==b.x && a.y==b.y; }
bool operator != (Vec a, Vec b) { return a.x!=b.x || a.y!=b.y; }
float mag2 (Vec a) { return a.x*a.x+a.y*a.y; }
float mag (Vec a) { return sqrt(mag2(a)); }
float dot (Vec a, Vec b) { return a.x*b.x + a.y*b.y; }
float ang (Vec a) { return atan2(a.y, a.x); }
float slope (Vec a) { return a.y / a.x; }
Vec norm (Vec a) { return a / mag(a); }
Vec rotcw (Vec a) { return Vec(a.y, -a.x); }
Vec rotccw (Vec a) { return Vec(-a.y, a.x); }

Vec polar (float r, float a) { return r*Vec(cos(a), sin(a)); }

bool defined (float a) { return a==a; }
bool defined (Vec a) { return defined(a.x) && defined(a.y); }

// Rectangles
struct Rect {
	float l;
	float b;
	float r;
	float t;
	Rect () { }
	Rect (float l, float b, float r, float t) :
		l(l), b(b), r(r), t(t)
	{ }
	Rect (Vec lb, Vec rt) :
		l(lb.x), b(lb.y), r(rt.x), t(rt.y)
	{ }
	Vec lb () const { return Vec(l, b); }
	Vec rt () const { return Vec(r, t); }
};

Rect operator & (const Rect& a, const Rect& b) {
	return Rect(
		MAX(a.l, b.l),
		MAX(a.b, b.b),
		MIN(a.r, b.r),
		MIN(a.t, b.t)
	);
}
Rect& operator &= (Rect& a, const Rect& b) { return a = a & b; }
bool defined (const Rect& r) { return r.l <= r.r && r.b <= r.t; }
Rect uninvert (Rect r) {
	if (r.l > r.r) { SWAP(r.l, r.r); }
	if (r.b > r.t) { SWAP(r.b, r.t); }
	return r;
}
bool in_rect (Vec p, Rect r) {
	return p.x >= r.l
	    && p.y >= r.b
	    && p.x <= r.r
	    && p.y <= r.t;
}
Vec size (const Rect& r) {
	return Vec(r.r - r.l, r.t - r.b);
}


 // Circles
struct Circle {
	Vec c;
	float r;
	Circle (Vec c, float r) : c(c), r(r) { }
};

bool in_circle (Vec p, Circle c) {
	if (c.r == 0) return true;
	return sign_f(c.r)*mag2(p - c.c) < c.r*c.r;
}


struct Line {
	Vec a;
	Vec b;
	Line () { }
	Line (Vec a, Vec b) : a(a), b(b) { }
};

bool vertical (const Line& l) { return l.a.x == l.b.x; }
bool horizontal (const Line& l) { return l.a.y == l.b.y; }
float slope (const Line& l) { return (l.b.y - l.a.y) / (l.b.x - l.a.x); }
bool verticalish (const Line& l) { return abs_f(l.b.y - l.a.y) > abs_f(l.b.x - l.a.x); }
float solvey (const Line& l, float x) {
	return l.a.y + slope(l) * (x - l.a.x);
}
float solvex (const Line& l, float y) {
	return l.a.x + (y - l.a.y) / slope(l);
}
Line bound_a (const Line& l) {
	return Line(l.a, l.a + rotcw(l.b - l.a));
}
Line bound_b (const Line& l) {
	return Line(l.b + rotccw(l.a - l.b), l.b);
}


bool across_line (Vec p, const Line& l) {
	if (vertical(l))
		return (l.a.y < l.b.y) ? p.x < l.a.x
		                       : p.x > l.a.x;
	float y = solvey(l, p.x);
	return (l.a.x < l.b.x) ? y < p.y
	                       : y > p.y;
}

Vec uncross_line (Vec p, Line l) {
	// To get the intersection of two lines, use
	// ay + asx = by + bsx (y is y-intercept and s is slope)
	// x = (by - ay) / (as - bs)
	// y = liney(a, b, x)
	if (vertical(l))
		return Vec(l.a.x, p.y);
	if (horizontal(l))
		return Vec(p.x, l.a.y);
	float x = (solvey(l, 0) - solvey(Line(p, p + rotccw(l.b - l.a)), 0))
	        / (slope(rotccw(l.b - l.a)) - slope(l));
//	printf("x = (%f-%f)/(%f-%f) = %f\n",
//		liney(a, b, 0), liney(p, p + rotcw(b - a), 0),
//		slope(rotcw(b - a)), slope(b - a),
//		x
//	);
	return Vec(x, solvey(l, x));
}

Vec uncross_circle (Vec p, Circle c) {
	if (c.r == 0) return c.c;
	return c.c + abs_f(c.r) * norm(p - c.c);
}

Vec intersect_lines (const Line& a, const Line& b) {
	if (vertical(a)) {
		if (vertical(b)) return Vec::undef;
		if (horizontal(b)) return Vec(a.a.x, b.a.y);
		return Vec(a.a.x, solvey(b, a.a.x));
	}
	if (horizontal(a)) {
		if (vertical(b)) return Vec(b.a.x, a.a.y);
		if (horizontal(b)) return Vec::undef;
		return Vec(solvex(b, a.a.y), a.a.y);
	}
	if (vertical(b)) return Vec(b.a.x, solvey(a, b.a.x));
	if (horizontal(b)) return Vec(solvex(a, b.a.y), b.a.y);
	float x = (solvey(a, 0) - solvey(b, 0))
	        / (slope(b) - slope(a));
	return Vec(x, solvey(a, x));
}

float line_fraction (Vec p, const Line& l) {
	if (verticalish(l)) return (p.y - l.a.y) / (l.b.y - l.a.y);
	else return (p.x - l.a.x) / (l.b.x - l.a.x);
}
bool in_line (Vec p, const Line& l) {
	if (verticalish(l)) {
		if (l.a.y < l.b.y) return p.y >= l.a.y && p.y <= l.b.y;
		else return p.y <= l.a.y && p.y >= l.b.y;
	}
	else {
		if (l.a.x < l.b.x) return p.x >= l.a.x && p.x <= l.b.x;
		else return p.x <= l.a.x && p.x >= l.b.x;
	}
}

Line intersect_line_circle (const Line& l, const Circle& c) {
	 // Formula take from http://mathworld.wolfram.com/Circle-LineIntersection.html
	 // with slight modifications.
	double dx = l.b.x - l.a.x;
	double dy = l.b.y - l.a.y;
	double dr2 = dx*dx + dy*dy;
	double D = (l.a.x - c.c.x) * (l.b.y - c.c.y) - (l.b.x - c.c.x) * (l.a.y - c.c.y);
	double disc = c.r*c.r * dr2 - D*D;
	if (disc < 0) return Line(Vec::undef, Vec::undef);
	if (disc == 0) {
		Vec tangent = c.c + Vec(
			D * dy / dr2,
			-D * dx / dr2
		);
		return Line(tangent, tangent);
	}

	// (disc > 0)
	double sqrtdisc = sqrt(disc);
	double variant_x = sign_f(dy) * dx * sqrtdisc;
	double variant_y = abs_f(dy) * sqrtdisc;
	Line r = Line(
		c.c + Vec(
			(D * dy - variant_x) / dr2,
			(-D * dx - variant_y) / dr2
		),
		c.c + Vec(
			(D * dy + variant_x) / dr2,
			(-D * dx + variant_y) / dr2
		)
	);
	if (abs_f(slope(r) - slope(l)) > 0.1)
		printf("Warning: something went wrong in intersect_line_circle.\n");
	return r;
}


Rect aabb (const Line& l) {
	return uninvert(Rect(l.a, l.b));
}
Rect aabb (const Circle& c) {
	float r = abs_f(c.r);
	return Rect(
		c.c.x - r, c.c.y - r, c.c.x + r, c.c.y + r
	);
}




