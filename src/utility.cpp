
#ifdef HEADER

#define STRINGIFY(v) #v
#define CE constexpr
typedef const char* CStr;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint32 uint;

 // Constants
const float FPS = 10.0;
const float FR = 1/10.0;
const float PX = 1/16.0;
const float PI = M_PI;

 // GENERIC NUMERIC
template <class A>
CE A max (A a, A b) { return a > b ? a : b; }
template <class A>
CE A min (A a, A b) { return a < b ? a : b; }
template <class A>
void swap (A& a, A& b) { A t = b; b = a; a = t; }

 // FLOAT
CE float f_mod (float a, float b) {
	return a >= 0 ? a - b*(double)(int)(a/b)
	              : a - b*(double)(int)(a/b);
}
CE float f_abs (float x) { return x>0 ? x : -x; }
CE int f_sign (float x) { return (x>0)-(x<0); }

float dither (float c, float err) {
	return c - err + rand()*2.0*err/RAND_MAX;
}

 // ANGLES
CE float ang_flip (float a) {
	return a<0 ? -PI-a : PI-a;
}
bool ang_ccw_of (float a, float b) {
	return b < 0 ? a > b && a < b+PI
	             : a > b || a < b-PI;
}
float ang_diff (float a, float b) {
	float diff = f_abs(a - b);
	if (diff > PI) diff = 2*PI - diff;
	return diff;
}

 // Using angles in animation
uint8 ang_to_frame (float a) {
	if (a != a) return 4;
	if (ang_ccw_of(a, PI/2)) {
		a = ang_flip(a);
	}
	return
	  a > PI * +7/16.0 ? 8
	: a > PI * +5/16.0 ? 7
	: a > PI * +3/16.0 ? 6
	: a > PI * +1/16.0 ? 5
	: a > PI * -1/16.0 ? 4
	: a > PI * -3/16.0 ? 3
	: a > PI * -5/16.0 ? 2
	: a > PI * -7/16.0 ? 1
	:                    0;
}

 // VECTORS
 // Our wheels are better.
struct Vec {
	float x;
	float y;
	CE Vec () :x(0/0.0), y(0/0.0) { }
	CE Vec (float x, float y) :x(x), y(y) { }
    Vec (b2Vec2 bv) :x(bv.x), y(bv.y) { }
    operator b2Vec2& () { return reinterpret_cast<b2Vec2&>(*this); }
    operator const b2Vec2& () const { return reinterpret_cast<const b2Vec2&>(*this); }

	CE Vec scale (Vec s) const { return Vec(x*s.x, y*s.y); }
	CE Vec scalex (float s) const { return Vec(s*x, y); }
	CE Vec scaley (float s) const { return Vec(x, s*y); }

	CE bool is_defined () const { return x==x || y==y; }
	CE float mag2 () const { return x*x + y*y; }
	CE float mag () const { return sqrt(mag2()); }
	CE float ang () const { return atan2(y, x); }
	CE float slope () const { return y/x; }
	CE Vec rotcw () const { return Vec(y, -x); }
	CE Vec rotccw () const { return Vec(-y, x); }
	CE Vec norm () const;
	static const Vec undef;
};
const Vec Vec::undef = Vec(0/0.0, 0/0.0);

CE Vec operator - (Vec a) { return Vec(-a.x, -a.y); }
CE Vec operator + (Vec a, Vec b) { return Vec(a.x+b.x, a.y+b.y); }
CE Vec operator - (Vec a, Vec b) { return Vec(a.x-b.x, a.y-b.y); }
CE Vec operator * (Vec a, float b) { return Vec(a.x*b, a.y*b); }
CE Vec operator * (float a, Vec b) { return Vec(a*b.x, a*b.y); }
CE Vec operator / (Vec a,  float b) { return Vec(a.x/b, a.y/b); }
Vec operator += (Vec& a, Vec b) { return a = a + b; }
Vec operator -= (Vec& a, Vec b) { return a = a - b; }
Vec operator *= (Vec& a, float b) { return a = a * b; }
Vec operator /= (Vec& a, float b) { return a = a / b; }
CE bool operator == (Vec a, Vec b) { return a.x==b.x && a.y==b.y; }
CE bool operator != (Vec a, Vec b) { return a.x!=b.x || a.y!=b.y; }
CE float dot (Vec a, Vec b) { return a.x*b.x + a.y*b.y; }

CE Vec Vec::norm () const { return *this / mag(); }
CE Vec polar (float r, float a) { return r*Vec(cos(a), sin(a)); }


// RECTANGLES
struct Rect {
	float l;
	float b;
	float r;
	float t;
	CE Rect () :
		l(0/0.0), b(0/0.0), r(0/0.0), t(0/0.0)
	{ }
	CE Rect (float l, float b, float r, float t) :
		l(l), b(b), r(r), t(t)
	{ }
	CE Rect (Vec lb, Vec rt) :
		l(lb.x), b(lb.y), r(rt.x), t(rt.y)
	{ }
	CE Vec lb () const { return Vec(l, b); }
	CE Vec rt () const { return Vec(r, t); }
	CE bool is_defined () const { return l==l && b==b && r==r && t==t; }
	CE bool is_proper () const { return l <= r && b <= t; }
	CE bool covers (Vec p) const {
		return p.x >= l
		    && p.y >= b
		    && p.x <= r
		    && p.y <= t;
	}
	CE Vec size () const {
		return Vec(r - l, t - b);
	}
	CE Rect uninvert () const {
		return Rect(
			(l > r) ? r : l,
			(b > t) ? t : b,
			(l > r) ? l : r,
			(b > t) ? b : t
		);
	}
};

CE Rect operator & (const Rect& a, const Rect& b) {
	return Rect(
		max(a.l, b.l),
		max(a.b, b.b),
		min(a.r, b.r),
		min(a.t, b.t)
	);
}
Rect& operator &= (Rect& a, const Rect& b) { return a = a & b; }


 // CIRCLES
struct Circle {
	Vec c;
	float r;
	CE Circle (Vec c, float r) : c(c), r(r) { }
	 // A circle with 0 radius is considered to be inverted
	 // because that works best for camera bounds.
	CE bool covers (Vec p) const {
		return r == 0
		    || f_sign(r)*(p - c).mag2() < r*r;
	}
	CE Vec contact (Vec p) const {
		return (r == 0) ? c
		                : c + f_abs(r) * (p - c).norm();
	}
	CE Rect aabb () const {
		return Rect(
			c.x - f_abs(r),
			c.y - f_abs(r),
			c.x + f_abs(r),
			c.y + f_abs(r)
		);
	}
};

 // LINES
struct Line {
	Vec a;
	Vec b;
	CE Line () : a(), b() { }
	CE Line (Vec a, Vec b) : a(a), b(b) { }
	CE bool is_vertical () const { return a.x == b.x; }
	CE bool is_horizontal () const { return a.y == b.y; }
	CE float slope () const { return (b.y - a.y) / (b.x - a.x); }
	CE bool is_verticalish () const { return f_abs(b.y - a.y) > f_abs(b.x - a.x); }
	CE float solvey (float x) const { return a.y + slope() * (x - a.x); }
	CE float solvex (float y) const { return a.x + (y - a.y) / slope(); }
	CE Line bound_a () const { return Line(a, a + (b - a).rotcw()); }
	CE Line bound_b () const { return Line(b + (a - b).rotccw(), b); }
	CE Rect aabb () const {
		return Rect(a, b).uninvert();
	}
	bool covers (Vec p) const {
		if (is_vertical())
			return (a.y < b.y) ? p.x < a.x
			                   : p.x > a.x;
		float y = solvey(p.x);
		return (a.x < b.x) ? y < p.y
		                   : y > p.y;
	}
	Vec contact (Vec p) const {
		// To get the intersection of two lines, use
		// ay + asx = by + bsx (y is y-intercept and s is slope)
		// x = (by - ay) / (as - bs)
		// y = liney(a, b, x)
		if (is_vertical())
			return Vec(a.x, p.y);
		if (is_horizontal())
			return Vec(p.x, a.y);
		float x = (solvey(0) - Line(p, p + (b - a).rotccw()).solvey(0))
		        / ((b - a).rotccw().slope() - slope());
	//	printf("x = (%f-%f)/(%f-%f) = %f\n",
	//		liney(a, b, 0), liney(p, p + rotcw(b - a), 0),
	//		slope(rotcw(b - a)), slope(b - a),
	//		x
	//	);
		return Vec(x, solvey(x));
	}
	float fraction (Vec p) const {
		if (is_verticalish()) return (p.y - a.y) / (b.y - a.y);
		else return (p.x - a.x) / (b.x - a.x);
	}
	bool contains (Vec p) const {
		if (is_verticalish()) {
			if (a.y < b.y) return p.y >= a.y && p.y <= b.y;
			else return p.y <= a.y && p.y >= b.y;
		}
		else {
			if (a.x < b.x) return p.x >= a.x && p.x <= b.x;
			else return p.x <= a.x && p.x >= b.x;
		}
	}
};



Vec operator & (const Line& a, const Line& b) {
	if (a.is_vertical()) {
		if (b.is_vertical()) return Vec::undef;
		if (b.is_horizontal()) return Vec(a.a.x, b.a.y);
		return Vec(a.a.x, b.solvey(a.a.x));
	}
	if (a.is_horizontal()) {
		if (b.is_vertical()) return Vec(b.a.x, a.a.y);
		if (b.is_horizontal()) return Vec::undef;
		return Vec(b.solvex(a.a.y), a.a.y);
	}
	if (b.is_vertical()) return Vec(b.a.x, a.solvey(b.a.x));
	if (b.is_horizontal()) return Vec(a.solvex(b.a.y), b.a.y);
	float x = (a.solvey(0) - b.solvey(0))
	        / (b.slope() - a.slope());
	return Vec(x, a.solvey(x));
}


Line operator & (const Line& l, const Circle& c) {
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
	double variant_x = f_sign(dy) * dx * sqrtdisc;
	double variant_y = f_abs(dy) * sqrtdisc;
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
	if (f_abs(r.slope() - l.slope()) > 0.1)
		dbg_util("Warning: something went wrong in intersect_line_circle.\n");
	return r;
}


 // VARIABLE WIDTH ARRAYS
template <class T>
struct VArray {
    uint n;
    T* p;
    CE VArray () :n(0), p(NULL) { }
    CE VArray (uint n, T* p) :n(n), p(p) { }

    CE T& operator [] (uint i) const { return p[i]; }

    //CE operator T* () const { return p; }
    CE operator uint () const { return n; }
    //CE operator bool () const { return n; }

//    void allocate (uint newn) {
//        n = newn;
//        p = new T [n];
//    }
//    void free () {
//        n = 0;
//        delete[] p;
//        p = 0;
//    }
//    void reallocate (uint newn) {
//        if (newn == n) return;
//        n = newn;
//        p = new (p) T [n];
//    }
};

 // Basic singly-linked list
template <class C>
struct Link {
    C head;
    Link<C>* tail;
    Link (C head, Link<C>* tail) : head(head), tail(tail) { }
};

 // CLASSES THAT AUTOMATICALLY POPULATE GLOBAL CONTAINERS

 // Store a pointer to the only one of this type.
template <class C>
struct Unique {
    static C* it;
    void activate () {
        if (it)
            printf("Error: Duplicate instantiation of Unique class.\n");
        it = static_cast<C*>(this); }
    void deactivate () {
        if (it != static_cast<C*>(this))
            printf("Error: Tried to deactivate non-active member of Unique class.\n");
        it = NULL;
    }
    Unique () { activate(); }
    ~Unique () { deactivate(); }
    Unique (bool active) { if (active) activate(); }
};
template <class C> C* Unique<C>::it = NULL;
 // the<Game_Settings>()
template <class C>
inline C* the () { return Unique<C>::it; }

 // Intrusive linked lists by class
template <class C>
struct Linked {
    C* next;
    C* prev;
    static C* first;
    static C* last;
    void activate () { 
       prev = last;
       next = NULL;
       prev->next = static_cast<C*>(this);

       last = static_cast<C*>(this);
       if (!first) first = static_cast<C*>(this);
    }
    void deactivate () {
        if (next) next->prev = prev;
        else if (last == static_cast<C*>(this)) last = prev;
        if (prev) prev->next = next;
        else if (first == static_cast<C*>(this)) first = next;
    }
    Linked () { activate(); }
    ~Linked () { deactivate(); }
    Linked (bool active) { if (active) activate(); }
};
template <class C> C* Linked<C>::first = NULL;
template <class C> C* Linked<C>::last = NULL;

#define FOR_LINKED(i, C) for (auto i = Linked<C>::first; i; i = i->next)


 // For code size we're just using one hash type
KHASH_MAP_INIT_STR(hash, void*);

 // Significantly simplify the interface to khash
 // Our usage patterns will assume no failures, hence the warnings.
template <class C>
struct Hash {
    khash_t(hash)* table;
    Hash () :table(kh_init(hash)) { }
    ~Hash () { kh_destroy(hash, table); }
    void insert (CStr name, C* val) {
        int r;
        auto iter = kh_put(hash, table, name, &r);
        if (r == 0)
            printf("Warning: Hash<?>::insert overwrote %s.\n", name);
        kh_val(table, iter) = val;
    }
    void remove (CStr name) {
        auto iter = kh_get(hash, table, name);
        if (iter == kh_end(table))
            printf("Warning: Hash<?>::remove did not find %s.\n", name);
        else kh_del(hash, table, iter);
    }
    C* lookup (CStr name) {
        auto iter = kh_get(hash, table, name);
        if (iter == kh_end(table)) {
            printf("Warning: Hash<?>::lookup did not find %s.\n", name);
            return NULL;
        }
        return (C*)(kh_val(table, iter));
    }
    inline khiter_t next_khiter (khiter_t iter) {
        while (!kh_exist(table, iter)) iter++;
        return iter;
    }
/*    CStr create_string () {
        CStr names [kh_size(table)];
        CStr vals [kh_size(table)];
        for (uint i = 0, auto iter = kh_begin(table); iter != kh_end(table); i++, iter = next_khiter(iter)) {
            names[i] = kh_key(table, iter); vals[i] = kh_val(table, iter)->create_string();
        }
        uint len = 0;
        for (uint i = 0; i < kh_size(table); i++) {
            len += strlen(names[i]) + 1 + strlen(vals[i]) + 1;
        }
        if (len) len--;
        char* r = malloc(len+1);
        uint ri = 0;
        for (uint i = 0; i < kh_size(table); i++) {
            if (i != 0) r[ri++] = ' ';
            for (uint ni = 0; names[i][ni]; ni++) r[ri++] = names[i][ni];
            r[ri++] = '=';
            for (uint vi = 0; vals[i][vi]; vi++) r[ri++] = vals[i][vi];
        }
        r[ri] = 0;
        for (uint i = 0; i < kh_size(table); i++) {
            delete vals[i];
        }
        return r;
    }*/
};

 // Keep a hash of every member of this class.
template <class C>
struct Hashed {
    static Hash<C> table;
    CStr name;
    void activate () { if (name) table.insert(name, static_cast<C*>(this)); }
    void deactivate () { if (name) table.remove(name); }
    void set_name (CStr newname) { deactivate(); name = newname; activate(); }
    Hashed (CStr name = NULL) :name(name) { activate(); }
    ~Hashed () { deactivate(); }
};
template <class C> Hash<C> Hashed<C>::table;


#define FOR_HASH(i, h) for (auto i##_iter = kh_begin((h).table), auto i = (C*)kh_val((h).table, i##_iter); i##_iter != kh_end((h).table); i##_iter = (h).next_khiter(i##_iter), i = (C*)kh_val((h).table, i##_iter))
#define FOR_HASHED(i, C) FOR_HASH(i, Hashed<C>::table)



#endif
