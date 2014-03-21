#ifndef HAVE_UTIL_GEOMETRY_H
#define HAVE_UTIL_GEOMETRY_H

#include <Box2D/Box2D.h>
#include "util/inc/honestly.h"

namespace util {

     // MISC
    CE bool defined (float a) { return a == a; }

    CE float ce_abs (float a) { return a < 0 ? -a : a; }

     // ANGLES
    float angle_diff (float, float);
     // For animation; returns 0..8 where 0 = down, 4 = forward, 8 = up
    uint8 angle_frame (float ang);

     // Our wheels are better.
    struct Vec {
        float x;
        float y;
        CE Vec () :x(NAN), y(NAN) { }
        CE Vec (float x, float y) :x(x), y(y) { }
        Vec (b2Vec2 bv) :x(bv.x), y(bv.y) { }
        operator b2Vec2& () { return reinterpret_cast<b2Vec2&>(*this); }
        operator const b2Vec2& () const { return reinterpret_cast<const b2Vec2&>(*this); }
    };

    CE Vec operator - (Vec a) { return Vec(-a.x, -a.y); }
    CE Vec operator + (Vec a, Vec b) { return Vec(a.x+b.x, a.y+b.y); }
    CE Vec operator - (Vec a, Vec b) { return Vec(a.x-b.x, a.y-b.y); }
    CE Vec operator * (Vec a, float b) { return Vec(a.x*b, a.y*b); }
    CE Vec operator * (float a, Vec b) { return Vec(a*b.x, a*b.y); }
     // Okay, okay.
    CE Vec operator * (Vec a, Vec b) { return Vec(a.x*b.x, a.y*b.y); }
    CE Vec operator / (Vec a,  float b) { return Vec(a.x/b, a.y/b); }
    static inline Vec operator += (Vec& a, Vec b) { return a = a + b; }
    static inline Vec operator -= (Vec& a, Vec b) { return a = a - b; }
    static inline Vec operator *= (Vec& a, float b) { return a = a * b; }
    static inline Vec operator /= (Vec& a, float b) { return a = a / b; }
    CE bool operator == (Vec a, Vec b) { return a.x==b.x && a.y==b.y; }
    CE bool operator != (Vec a, Vec b) { return a.x!=b.x || a.y!=b.y; }
    CE bool defined (Vec a) { return defined(a.x) && defined(a.y); }
    CE float length2 (Vec a) { return a.x*a.x + a.y*a.y; }
    static inline float length (Vec a) { return sqrt(length2(a)); }
    static inline float distance (Vec a, Vec b) { return length(b - a); }
    static inline float angle (Vec a) { return atan2(a.y, a.x); }
    CE float slope (Vec a) { return a.y/a.x; }
    CE Vec rotcw (Vec a) { return Vec(a.y, -a.x); }
    CE Vec rotccw (Vec a) { return Vec(-a.y, a.x); }
    CE float dot (Vec a, Vec b) { return a.x*b.x + a.y*b.y; }
    static inline Vec normalize (Vec a) { return a / length(a); }
    static inline Vec polar (float r, float a) { return r*Vec(cos(a), sin(a)); }

    // RECTANGLES
    struct Rect {
        float l;
        float b;
        float r;
        float t;
        CE Rect () : l(NAN), b(NAN), r(NAN), t(NAN) { }
        CE Rect (float l, float b, float r, float t) :
            l(l), b(b), r(r), t(t)
        { }
        CE Rect (Vec lb, Vec rt) :
            l(lb.x), b(lb.y), r(rt.x), t(rt.y)
        { }
        CE Vec lb () const { return Vec(l, b); }
        CE Vec rb () const { return Vec(r, b); }
        CE Vec rt () const { return Vec(r, t); }
        CE Vec lt () const { return Vec(l, t); }
        CE bool covers (Vec p) const {
            return p.x >= l
                && p.y >= b
                && p.x <= r
                && p.y <= t;
        }
        CE Vec size () const {
            return Vec(r - l, t - b);
        }
        CE Vec center () const { return lb() + rt() / 2; }
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
            a.l > b.l ? a.l : b.l,
            a.b > b.b ? a.b : b.b,
            a.r < b.r ? a.r : b.r,
            a.t < b.t ? a.t : b.t
        );
    }
    static inline Rect& operator &= (Rect& a, const Rect& b) { return a = a & b; }

    CE Rect operator + (const Rect& a, Vec b) { return Rect(a.l+b.x, a.b+b.y, a.r+b.x, a.t+b.y); }
    CE Rect operator + (Vec a, const Rect& b) { return b+a; }
    CE Rect operator - (const Rect& a, Vec b) { return Rect(a.l-b.x, a.b-b.y, a.r-b.x, a.t-b.y); }
    CE bool defined (const Rect& a) { return defined(a.l) && defined(a.b) && defined(a.r) && defined(a.t); }
    CE bool proper (const Rect& a) { return a.l <= a.r && a.b <= a.t; }
    CE Rect bounds (const Rect& a) { return a; }
    CE bool contains (const Rect& r, Vec p) {
        return p.x > r.l
            && p.y > r.b
            && p.x < r.t
            && p.y < r.r;
    }
    CE bool covers (const Rect& r, Vec p) {
        return p.x >= r.l
            && p.y >= r.b
            && p.x <= r.t
            && p.y <= r.r;
    }
    CE Vec constrain (const Rect& r, Vec p) {
        return Vec(
            p.x < r.l ? r.l : p.x > r.r ? r.r : p.x,
            p.y < r.b ? r.b : p.y > r.t ? r.t : p.y
        );
    }

     // Circles.  These can be inverted if their radius is negative.
    struct Circle {
        Vec c;
        float r;
        CE Circle () : c(),  r(NAN) { }
        CE Circle (Vec c, float r) : c(c), r(r) { }
    };

    CE Circle operator + (const Circle& a, Vec b) { return Circle(a.c+b, a.r); }
    CE Circle operator + (Vec a, const Circle& b) { return b+a; }
    CE Circle operator - (const Circle& a, Vec b) { return Circle(a.c-b, a.r); }
    CE bool defined (const Circle& a) { return defined(a.c) && defined(a.r); }
    CE bool proper (const Circle& a) { return a.r >= 0; }
    CE Rect bounds (const Circle& a) {
        return Rect(a.c.x - a.r, a.c.y - a.r, a.c.x + a.r, a.c.y + a.r);
    }
    CE bool contains (const Circle& c, Vec p) {
        return c.r >= 0
            ? length2(p - c.c) < c.r*c.r
            : length2(p - c.c) >= c.r*c.r;
    }
    CE bool covers (const Circle& c, Vec p) {
        return c.r >= 0
            ? length2(p - c.c) <= c.r*c.r
            : length2(p - c.c) > c.r*c.r;
    }
    static inline Vec snap (const Circle& c, Vec p) {
        return c.c + normalize(p - c.c) * fabs(c.r);
    }

     // These act at times like infinite lines and at times like segments
    struct Line {
        Vec a;
        Vec b;
        CE Line () : a(), b() { }
        CE Line (Vec a, Vec b) : a(a), b(b) { }

        CE float y_at_x (float x) { return a.y + (x - a.x) * slope(b - a); }
        CE float x_at_y (float y) { return a.x + (y - a.y) / slope(b - a); }
    };

    CE Line operator + (const Line& a, Vec b) { return Line(a.a+b, a.b+b); }
    CE Line operator + (Vec a, const Line& b) { return Line(a+b.a, a+b.b); }
    CE Line operator - (const Line& a, Vec b) { return Line(a.a-b, a.b-b); }
    CE bool defined (const Line& l) { return defined(l.a) && defined(l.b); }
    CE float slope (const Line& l) { return slope(l.b - l.a); }
    CE bool vertical (const Line& l) { return l.a.x == l.b.x; }
    CE bool horizontal (const Line& l) { return l.a.y == l.b.y; }
    CE bool verticalish (const Line& l) { return ce_abs(l.b.y - l.a.y) > ce_abs(l.b.x - l.a.x); }
    CE bool horizontalish (const Line& l) { return !verticalish(l); }
    CE Rect bounds (const Line& l) { return Rect(l.a, l.b).uninvert(); }
    CE Line bound_a (const Line& l) {
        return Line(l.a + rotccw(l.b - l.a), l.a);
    }
    CE Line bound_b (const Line& l) {
        return Line(l.b, l.b + rotccw(l.b - l.a));
    }
     // The area covered by the line is half of all 2-D space;
     //  specifically the side consistent with CCW polygon winding.
    CE bool contains (const Line& l, Vec p) {
        return vertical(l)
            ? l.a.y < l.b.y ? p.x < l.a.x
                            : p.x > l.a.x
            : l.a.x < l.b.x ? l.y_at_x(p.x) < p.y
                            : l.y_at_x(p.x) > p.y;
    }
    CE bool covers (const Line& l, Vec p) {
        return vertical(l)
            ? l.a.y < l.b.y ? p.x <= l.a.x
                            : p.x >= l.a.x
            : l.a.x < l.b.x ? l.y_at_x(p.x) <= p.y
                            : l.y_at_x(p.x) >= p.y;
    }

     // Find the point at which two lines cross
    Vec intersect (const Line& a, const Line& b);

     // Find the two points (if any) at which these shapes touch
    Line intersect (const Circle& a, const Line& b);

     // Find a line tangent to both circles counterclockwise.
     //      v this line v
     //      _____________
     // b-> O             O <-a
    Line double_tangent (const Circle& a, const Circle& b);

    static inline Vec snap (const Line& l, Vec p) {
        return intersect(l, p + bound_a(l) - l.a);
    }

}

#endif
