#include "../inc/geometry.h"

namespace util {

    float angle_diff (float a, float b) {
        float d1 = fabs(a - b);
        return d1 > PI ? 2*PI - d1 : d1;
    }

    Vec constrain (Vec p, Rect& r) {
        return Vec(
            p.x < r.l ? r.l : p.x > r.r ? r.r : p.x,
            p.y < r.b ? r.b : p.x > r.t ? r.t : p.y
        );
    }

} using namespace util;


 // Let's just have Vecs and Rects be array-like, I think.
HCB_BEGIN(Vec)
    name("Vec");
    elem(member(&Vec::x));
    elem(member(&Vec::y));
HCB_END(Vec)

HCB_BEGIN(Rect)
    name("Rect");
    elem(member(&Rect::l));
    elem(member(&Rect::b));
    elem(member(&Rect::r));
    elem(member(&Rect::t));
HCB_END(Rect)
