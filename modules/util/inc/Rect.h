#ifndef HAVE_UTIL_RECT_H
#define HAVE_UTIL_RECT_H

#include "Vec.h"

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

HCB_BEGIN(Rect)
    elem(member(&Rect::l));
    elem(member(&Rect::b));
    elem(member(&Rect::r));
    elem(member(&Rect::t));
HCB_END(Rect)

#endif
