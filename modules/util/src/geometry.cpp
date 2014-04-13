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

    Vec intersect (const Line& a, const Line& b) {
        if (vertical(a)) {
            if (vertical(b)) return Vec();
            if (horizontal(b)) return Vec(a.a.x, b.a.y);
            return Vec(a.a.x, b.y_at_x(a.a.x));
        }
        if (horizontal(a)) {
            if (vertical(b)) return Vec(b.a.x, a.a.y);
            if (horizontal(b)) return Vec();
            return Vec(b.x_at_y(a.a.y), a.a.y);
        }
        if (vertical(b)) return Vec(b.a.x, a.y_at_x(b.a.x));
        if (horizontal(b)) return Vec(a.x_at_y(b.a.y), b.a.y);
        float x = (a.y_at_x(0) - b.y_at_x(0))
                / (slope(b) - slope(a));
        return Vec(x, a.y_at_x(x));
    }

    float sign (float a) { return (a > 0) - (a < 0); }

    Line intersect (const Circle& c, const Line& l) {
         // This seems to have better numeric stability than the standard algorithm.
         // Snapping the center to the line gives the midpoint between the two intersections.
        Vec nearest = snap(l, c.c);
        float dist2 = length2(nearest - c.c);
        if (dist2 > c.r*c.r)
            return Line();
        if (dist2 == c.r*c.r)
            return Line(nearest, nearest);
        float offset = sqrt(c.r*c.r - dist2);
        Vec direction = normalize(l.b - l.a);
        return nearest + Line(-offset * direction, offset * direction);
    }

    Line double_tangent (const Circle& a, const Circle& b) {
        if (a.r == 0 && b.r == 0) {
            return Line(a.c, b.c);
        }
        else {
            float ang = angle(b.c - a.c) - acos((a.r - b.r) / length(b.c - a.c));
            return Line(a.c + polar(a.r, ang), b.c + polar(b.r, ang));
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
