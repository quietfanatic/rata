


 // Our wheels are better.
struct Vec {
	float x;
	float y;
	inline Vec () { }
	inline Vec (float x, float y) :x(x), y(y) { }
	inline Vec (b2Vec2 v) :x(v.x), y(v.y) { }
	inline Vec (sf::Vector2<float> v) :x(v.x), y(v.y) { }
	operator const b2Vec2 () const { return b2Vec2(x, y); }
	operator const sf::Vector2<float> () const { return sf::Vector2<float>(x, y); }

	inline Vec scalex (float s) const { return Vec(s*x, y); }
	inline Vec scaley (float s) const { return Vec(x, s*y); }

	static const Vec undef;
};
const Vec Vec::undef = Vec(0/0.0, 0/0.0);


const inline Vec operator - (Vec a) { return Vec(-a.x, -a.y); }
const inline Vec operator + (Vec a, Vec b) { return Vec(a.x+b.x, a.y+b.y); }
const inline Vec operator - (Vec a, Vec b) { return Vec(a.x-b.x, a.y-b.y); }
const inline Vec operator * (Vec a, float b) { return Vec(a.x*b, a.y*b); }
const inline Vec operator * (float a, Vec b) { return Vec(a*b.x, a*b.y); }
const inline Vec operator / (Vec a,  float b) { return Vec(a.x/b, a.y/b); }
inline Vec operator += (Vec& a, Vec b) { return a = a + b; }
inline Vec operator -= (Vec& a, Vec b) { return a = a - b; }
inline Vec operator *= (Vec& a, float b) { return a = a * b; }
inline Vec operator /= (Vec& a, float b) { return a = a / b; }
inline bool operator == (Vec a, Vec b) { return a.x==b.x && a.y==b.y; }
inline bool operator != (Vec a, Vec b) { return a.x!=b.x || a.y!=b.y; }
const inline float mag2 (Vec a) { return a.x*a.x+a.y*a.y; }
const inline float mag (Vec a) { return sqrt(mag2(a)); }
const inline float dot (Vec a, Vec b) { return a.x*b.x + a.y*b.y; }
const inline float ang (Vec a) { return atan2(a.y, a.x); }
const inline Vec norm (Vec a) { return a / mag(a); }

const inline bool defined (float a) { return a==a; }
const inline bool defined (Vec a) { return defined(a.x) && defined(a.y); }


const inline bool in_rect (Vec a, Vec b, Vec c) {
	return a.x >= b.x
	    && a.x <= c.x
	    && a.y >= b.y
	    && a.y <= c.y;
}


