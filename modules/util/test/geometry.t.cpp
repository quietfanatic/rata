#include "util/inc/geometry.h"

#include "tap/inc/tap.h"
using namespace util;
using namespace tap;

tap::Tester geometry_tester ("util/geometry", [](){
    plan(16);
    ok(covers(Circle(Vec(2, 2), 2), Vec(1, 1)), "covers(Circle) can report true");
    ok(!covers(Circle(Vec(2, 2), 2), Vec(0, 1)), "covers(Circle) can report false");
    ok(covers(Line(Vec(0, 1), Vec(3, 1)), Vec(2, 0)), "covers(Line) horizontal true");
    ok(!covers(Line(Vec(0, 1), Vec(3, 1)), Vec(2, 2)), "covers(Line) horizontal false");
    ok(covers(Line(Vec(1, 0), Vec(1, 3)), Vec(0, 2)), "covers(Line) vertical true");
    ok(!covers(Line(Vec(1, 0), Vec(1, 3)), Vec(2, 2)), "covers(Line) vertical false");
    ok(covers(Line(Vec(0, 0), Vec(5, 5)), Vec(0, 1)), "covers(Line) diagonal true");
    ok(!covers(Line(Vec(0, 0), Vec(5, 5)), Vec(1, 0)), "covers(Line) diagonal false");
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
