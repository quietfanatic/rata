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
         // Formula taken from http://mathworld.wolfram.com/Circle-LineIntersection.html
         //  with slight modifications.
        double dx = l.b.x - l.a.x;
        double dy = l.b.y - l.a.y;
        double dr2 = dx*dx + dy*dy;
        double D = (l.a.x - c.c.x) * (l.b.y - c.c.y) - (l.b.x - c.c.x) * (l.a.y - c.c.y);
        double disc = c.r*c.r * dr2 - D*D;
        if (disc < 0) return Line();
        if (disc == 0) {
            Vec tangent = c.c + Vec(
                D * dy / dr2,
                -D * dx / dr2
            );
            return Line(tangent, tangent);
        }

        // (disc > 0)
        double sqrtdisc = sqrt(disc);
        double variant_x = sign(dy) * dx * sqrtdisc;
        double variant_y = abs(dy) * sqrtdisc;
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
        if (abs(slope(r) - slope(l)) > 0.1)
            fprintf(stderr, "Warning: something went wrong in intersect(circle, Line).\n");
        return r;
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
