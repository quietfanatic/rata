


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


static const inline Vec operator - (Vec a) { return Vec(-a.x, -a.y); }
static const inline Vec operator + (Vec a, Vec b) { return Vec(a.x+b.x, a.y+b.y); }
static const inline Vec operator - (Vec a, Vec b) { return Vec(a.x-b.x, a.y-b.y); }
static const inline Vec operator * (Vec a, float b) { return Vec(a.x*b, a.y*b); }
static const inline Vec operator * (float a, Vec b) { return Vec(a*b.x, a*b.y); }
static const inline Vec operator / (Vec a,  float b) { return Vec(a.x/b, a.y/b); }
static inline Vec operator += (Vec& a, Vec b) { return a = a + b; }
static inline Vec operator -= (Vec& a, Vec b) { return a = a - b; }
static inline Vec operator *= (Vec& a, float b) { return a = a * b; }
static inline Vec operator /= (Vec& a, float b) { return a = a / b; }
static inline bool operator == (Vec a, Vec b) { return a.x==b.x && a.y==b.y; }
static inline bool operator != (Vec a, Vec b) { return a.x!=b.x || a.y!=b.y; }
static const inline float mag2 (Vec a) { return a.x*a.x+a.y*a.y; }
static const inline float mag (Vec a) { return sqrt(mag2(a)); }
static const inline float dot (Vec a, Vec b) { return a.x*b.x + a.y*b.y; }
static const inline float ang (Vec a) { return atan2(a.y, a.x); }
static const inline Vec norm (Vec a) { return a / mag(a); }

static const inline bool defined (float a) { return a==a; }
static const inline bool defined (Vec a) { return defined(a.x) && defined(a.y); }


static const inline bool in_rect (Vec a, Vec b, Vec c) {
	return a.x >= b.x
	    && a.x <= c.x
	    && a.y >= b.y
	    && a.y <= c.y;
}


