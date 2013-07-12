#ifndef HAVE_UTIL_GEOMETRY_H
#define HAVE_UTIL_GEOMETRY_H

#include <Box2D/Box2D.h>
#include "honestly.h"
#include "../../hacc/inc/haccable.h"

namespace util {

     // ANGLES
    float angle_diff (float, float);
     // For animation; returns 0..8 where 0 = down, 4 = forward, 8 = up
    uint8 angle_frame (float ang);

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
    };

    CE inline Vec operator - (Vec a) { return Vec(-a.x, -a.y); }
    CE inline Vec operator + (Vec a, Vec b) { return Vec(a.x+b.x, a.y+b.y); }
    CE inline Vec operator - (Vec a, Vec b) { return Vec(a.x-b.x, a.y-b.y); }
    CE inline Vec operator * (Vec a, float b) { return Vec(a.x*b, a.y*b); }
    CE inline Vec operator * (float a, Vec b) { return Vec(a*b.x, a*b.y); }
    CE inline Vec operator / (Vec a,  float b) { return Vec(a.x/b, a.y/b); }
    static inline Vec operator += (Vec& a, Vec b) { return a = a + b; }
    static inline Vec operator -= (Vec& a, Vec b) { return a = a - b; }
    static inline Vec operator *= (Vec& a, float b) { return a = a * b; }
    static inline Vec operator /= (Vec& a, float b) { return a = a / b; }
    CE inline bool operator == (Vec a, Vec b) { return a.x==b.x && a.y==b.y; }
    CE inline bool operator != (Vec a, Vec b) { return a.x!=b.x || a.y!=b.y; }
    CE inline float dot (Vec a, Vec b) { return a.x*b.x + a.y*b.y; }

    CE inline Vec Vec::norm () const { return *this / mag(); }
    CE inline Vec polar (float r, float a) { return r*Vec(cos(a), sin(a)); }

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
        CE Vec rb () const { return Vec(r, b); }
        CE Vec rt () const { return Vec(r, t); }
        CE Vec lt () const { return Vec(l, t); }
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

    Vec constrain (Vec p, const Rect& range);

}

#endif
