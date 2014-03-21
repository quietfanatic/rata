#include "util/inc/geometry.h"

#include "tap/inc/tap.h"
using namespace util;
using namespace tap;

tap::Tester geometry_tester ("util/geometry", [](){
    plan(33);
    Rect a = Rect(0, 0, 2, 2) & Rect(1, 1, 3, 3);
    is(a.l, 1.f, "Rect & Rect l 1");
    is(a.b, 1.f, "Rect & Rect b 1");
    is(a.r, 2.f, "Rect & Rect r 1");
    is(a.t, 2.f, "Rect & Rect t 1");
    a = Rect(1, 1, 3, 3) & Rect(0, 0, 2, 2);
    is(a.l, 1.f, "Rect & Rect l 2");
    is(a.b, 1.f, "Rect & Rect b 2");
    is(a.r, 2.f, "Rect & Rect r 2");
    is(a.t, 2.f, "Rect & Rect t 2");
    ok(contains(Rect(12, -4, 32, 11), Vec(20, 7.5)), "previously troublesome contains(Rect) works");
    ok(verticalish(Line(Vec(0, 0), Vec(1, 2))), "verticalish(Line) can report true");
    ok(!verticalish(Line(Vec(0, 0), Vec(2, 1))), "verticalish(Line) can report false");
    ok(contains(Circle(Vec(2, 2), 2), Vec(1, 1)), "contains(Circle) can report true");
    ok(!contains(Circle(Vec(2, 2), 2), Vec(0, 1)), "contains(Circle) can report false");
    ok(contains(Line(Vec(0, 1), Vec(3, 1)), Vec(2, 2)), "contains(Line) horizontal true");
    ok(!contains(Line(Vec(0, 1), Vec(3, 1)), Vec(2, 0)), "contains(Line) horizontal false");
    ok(contains(Line(Vec(1, 0), Vec(1, 3)), Vec(0, 2)), "contains(Line) vertical true");
    ok(!contains(Line(Vec(1, 0), Vec(1, 3)), Vec(2, 2)), "contains(Line) vertical false");
    ok(contains(Line(Vec(0, 0), Vec(5, 5)), Vec(0, 1)), "contains(Line) diagonal true");
    ok(!contains(Line(Vec(0, 0), Vec(5, 5)), Vec(1, 0)), "contains(Line) diagonal false");
    ok(!contains(Line(Vec(5, 5), Vec(0, 0)), Vec(0, 1)), "contains(Line) diagonal false");
    ok(contains(Line(Vec(5, 5), Vec(0, 0)), Vec(1, 0)), "contains(Line) diagonal true");
    ok(contains(Line(Vec(0, 0), Vec(5, 0)), Vec(0, 1)), "contains(Line) horizontal true");
    ok(!contains(Line(Vec(0, 0), Vec(5, 0)), Vec(0, -1)), "contains(Line) horizontal false");
    ok(contains(Line(Vec(0, 0), Vec(0, 5)), Vec(-1, 0)), "contains(Line) vertical true");
    ok(!contains(Line(Vec(0, 0), Vec(0, 5)), Vec(1, 0)), "contains(Line) vertical false");
    Line ba = bound_a(Line(Vec(0, 0), Vec(5, 5)));
    is(ba.a, Vec(-5, 5), "bound_a(Line) seems to work (a)");
    is(ba.b, Vec(0, 0), "bound_a(Line) seems to work (b)");
    Line bb = bound_b(Line(Vec(0, 0), Vec(5, 5)));
    is(bb.a, Vec(5, 5), "bound_b(Line) seems to work (a)");
    is(bb.b, Vec(0, 10), "bound_b(Line) seems to work (b)");
    Vec snapped = snap(Line(Vec(0, 0), Vec(5, 5)), Vec(2, 0));
    within(snapped.x, 0.01, 1, "snap to Line seems to work (x)");
    within(snapped.y, 0.01, 1, "snap to Line seems to work (y)");
    snapped = snap(Circle(Vec(1, 1), 1), Vec(3, 1));
    within(snapped.x, 0.01, 2, "snap to Circle seems to work (x)");
    within(snapped.y, 0.01, 1, "snap to Circle seems to work (y)");
});
