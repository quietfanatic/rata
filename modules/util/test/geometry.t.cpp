#include "util/inc/geometry.h"

#include "tap/inc/tap.h"
using namespace util;
using namespace tap;

tap::Tester geometry_tester ("util/geometry", [](){
    plan(8);
    ok(Circle(Vec(2, 2), 2).covers(Vec(1, 1)), "Circle::covers can report true");
    ok(!Circle(Vec(2, 2), 2).covers(Vec(0, 1)), "Circle::covers can report false");
    ok(Line(Vec(0, 1), Vec(3, 1)).covers(Vec(2, 0)), "Line::covers horizontal true");
    ok(!Line(Vec(0, 1), Vec(3, 1)).covers(Vec(2, 2)), "Line::covers horizontal false");
    ok(Line(Vec(1, 0), Vec(1, 3)).covers(Vec(0, 2)), "Line::covers vertical true");
    ok(!Line(Vec(1, 0), Vec(1, 3)).covers(Vec(2, 2)), "Line::covers vertical false");
    ok(Line(Vec(0, 0), Vec(5, 5)).covers(Vec(0, 1)), "Line::covers diagonal true");
    ok(!Line(Vec(0, 0), Vec(5, 5)).covers(Vec(1, 0)), "Line::covers diagonal false");
});
