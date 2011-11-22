


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


const Vec operator - (Vec a) { return Vec(-a.x, -a.y); }
const Vec operator + (Vec a, Vec b) { return Vec(a.x+b.x, a.y+b.y); }
const Vec operator - (Vec a, Vec b) { return Vec(a.x-b.x, a.y-b.y); }
const Vec operator * (Vec a, float b) { return Vec(a.x*b, a.y*b); }
const Vec operator * (float a, Vec b) { return Vec(a*b.x, a*b.y); }
const Vec operator / (Vec a,  float b) { return Vec(a.x/b, a.y/b); }
Vec operator += (Vec& a, Vec b) { return a = a + b; }
Vec operator -= (Vec& a, Vec b) { return a = a - b; }
Vec operator *= (Vec& a, float b) { return a = a * b; }
Vec operator /= (Vec& a, float b) { return a = a / b; }
bool operator == (Vec a, Vec b) { return a.x==b.x && a.y==b.y; }
bool operator != (Vec a, Vec b) { return a.x!=b.x || a.y!=b.y; }
const float mag2 (Vec a) { return a.x*a.x+a.y*a.y; }
const float mag (Vec a) { return sqrt(mag2(a)); }
const float dot (Vec a, Vec b) { return a.x*b.x + a.y*b.y; }
const float ang (Vec a) { return atan2(a.y, a.x); }
const Vec norm (Vec a) { return a / mag(a); }

const bool defined (float a) { return a==a; }
const bool defined (Vec a) { return defined(a.x) && defined(a.y); }


const bool in_rect (Vec a, Vec b, Vec c) {
	return a.x >= b.x
	    && a.x <= c.x
	    && a.y >= b.y
	    && a.y <= c.y;
}


