


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
float liney (Vec a, Vec b, float x) {
	return a.y + slope(b - a) * (x - a.x);
}
Vec norm (Vec a) { return a / mag(a); }
Vec rotcw (Vec a) { return Vec(-a.y, a.x); }
Vec rotccw (Vec a) { return Vec(a.y, -a.x); }

Vec polar (float r, float a) { return r*Vec(cos(a), sin(a)); }

bool defined (float a) { return a==a; }
bool defined (Vec a) { return defined(a.x) && defined(a.y); }


// Geometry

bool in_rect (Vec p, Vec a, Vec b) {
	return p.x >= a.x
	    && p.x <= b.x
	    && p.y >= a.y
	    && p.y <= b.y;
}

bool across_line (Vec p, Vec a, Vec b) {
	if (a.x == b.x)
		return (a.y < b.y) ? p.x < a.x
		                   : p.x > a.x;
	float y = liney(a, b, p.x);
	return (a.x < b.x) ? y < p.y
	                   : y > p.y;
}

Vec uncross_line (Vec p, Vec a, Vec b) {
	// To get the intersection of two lines, use
	// ay + asx = by + bsx (y is y-intercept and s is slope)
	// x = (by - ay) / (as - bs)
	// y = liney(a, b, x)
	if (a.x == b.x)
		return Vec(a.x, p.y);
	if (a.y == b.y)
		return Vec(p.x, a.y);
	float x = (liney(a, b, 0) - liney(p, p + rotcw(b - a), 0))
	        / (slope(rotcw(b - a)) - slope(b - a));
//	printf("x = (%f-%f)/(%f-%f) = %f\n",
//		liney(a, b, 0), liney(p, p + rotcw(b - a), 0),
//		slope(rotcw(b - a)), slope(b - a),
//		x
//	);
	return Vec(x, liney(a, b, x));
}


