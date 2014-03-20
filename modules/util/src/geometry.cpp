#include "util/inc/geometry.h"

#include "hacc/inc/haccable.h"

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

    Vec Circle::snap (Vec p) {
        return p.normalize() * fabs(r);
    }

    Vec Line::snap (Vec p) {
        return intersect(*this, p + bound_a() - a);
    }

    Vec intersect (const Line& a, const Line& b) {
        if (a.vertical()) {
            if (b.vertical()) return Vec();
            if (b.horizontal()) return Vec(a.a.x, b.a.y);
            return Vec(a.a.x, b.y_at_x(a.a.x));
        }
        if (a.horizontal()) {
            if (b.vertical()) return Vec(b.a.x, a.a.y);
            if (b.horizontal()) return Vec();
            return Vec(b.x_at_y(a.a.y), a.a.y);
        }
        if (b.vertical()) return Vec(b.a.x, a.y_at_x(b.a.x));
        if (b.horizontal()) return Vec(a.x_at_y(b.a.y), b.a.y);
        float x = (a.y_at_x(0) - b.y_at_x(0))
                / (b.slope() - a.slope());
        return Vec(x, a.y_at_x(x));
    }

    Line double_tangent (const Circle& a, const Circle& b) {
        if (a.r == 0 && b.r == 0) {
            return Line(a.c, b.c);
        }
        else {
            float angle = (b.c - a.c).ang() - acos((a.r - b.r) / (b.c - a.c).mag());
            return Line(a.c + polar(a.r, angle), b.c + polar(b.r, angle));
        }
    }

} using namespace util;


 // Let's just have Vecs and Rects be array-like, I think.
HACCABLE(Vec) {
    name("util::Vec");
    elem(member(&Vec::x));
    elem(member(&Vec::y));
}

HACCABLE(Rect) {
    name("util::Rect");
    elem(member(&Rect::l));
    elem(member(&Rect::b));
    elem(member(&Rect::r));
    elem(member(&Rect::t));
}

HACCABLE(Circle) {
    name("util::Circle");
    elem(member(&Circle::c));
    elem(member(&Circle::r));
}

HACCABLE(Line) {
    name("util::Line");
    elem(member(&Line::a));
    elem(member(&Line::b));
}
