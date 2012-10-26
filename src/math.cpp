
#ifdef HEADER

Debugger math_dbg ("math");

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
    //    printf("x = (%f-%f)/(%f-%f) = %f\n",
    //        liney(a, b, 0), liney(p, p + rotcw(b - a), 0),
    //        slope(rotcw(b - a)), slope(b - a),
    //        x
    //    );
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
        math_dbg.dbg("Something went wrong in intersect_line_circle.\n");
    return r;
}



#endif
