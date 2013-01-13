
#include <Box2D/Box2D.h>
#include "honestly.h"
#include "../../hacc/inc/haccable.h"

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

 // Let's just have vecs be array-like, I think.
HCB_BEGIN(Vec)
    elem(member(&Vec::x));
    elem(member(&Vec::y));
HCB_END(Vec)

