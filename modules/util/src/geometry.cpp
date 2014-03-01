#include "util/inc/geometry.h"

namespace util {

    float angle_diff (float a, float b) {
        float d1 = fabs(a - b);
         // In case the angles were not normalized
        if (d1 > 2*PI) d1 = fmod(d1, 2*PI);
        return d1 > PI ? 2*PI - d1 : d1;
    }

    uint8 angle_frame (float ang) {
        float up_ang = angle_diff(ang, -PI/2);
        return lround(up_ang / PI * 8);
    }

    Vec constrain (Vec p, const Rect& r) {
        return Vec(
            p.x < r.l ? r.l : p.x > r.r ? r.r : p.x,
            p.y < r.b ? r.b : p.y > r.t ? r.t : p.y
        );
    }

} using namespace util;


 // Let's just have Vecs and Rects be array-like, I think.
HACCABLE(Vec) {
    name("Vec");
    elem(member(&Vec::x));
    elem(member(&Vec::y));
}

HACCABLE(Rect) {
    name("Rect");
    elem(member(&Rect::l));
    elem(member(&Rect::b));
    elem(member(&Rect::r));
    elem(member(&Rect::t));
}
