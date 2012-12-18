
#include "../inc/haccable.h"

using namespace hacc;

template <> struct Haccable<int> : Haccability<int> {
    void describe (Haccer& h, int& it) {
        h.as_integer(it);
    }
};
template <> struct Haccable<float> : Haccability<float> {
    void info () {
        hacctype("float");
        hacctype("Float");
    }
    void describe (Haccer& h, float& it) {
        h.as_float(it);
    }
};

struct Vectorly {
    float x;
    float y;
    bool operator == (Vectorly o) const { return x==o.x && y==o.y; }
};

Vectorly vy1 { 1, 2 };
Vectorly vy2 { 3, 4 };

template <> struct Haccable<Vectorly> : Haccability<Vectorly> {
    String haccid (const Vectorly& vy) {
        if (&vy == &vy1) return "vy1";
        else if (&vy == &vy2) return "vy2";
        else return "";
    }
    Vectorly* find_by_haccid (String id) {
        if (id == "vy1") return &vy1;
        else if (id == "vy2") return &vy2;
        else return null;
    }
    void describe (Haccer& h, Vectorly& it) {
        h.elem(it.x, 0.f);
        h.elem(it.y, 0.f);
        h.attr("x", it.x, 0.f);
        h.attr("y", it.y, 0.f);
    }
};

struct MyFloat {
    float val;
    operator float () const { return val; }
    MyFloat (float val) : val(val) { }
    MyFloat () { }
    bool operator == (MyFloat o) const { return val==o.val; }
};
template <> struct Haccable<MyFloat> : Haccability<MyFloat> {
    void describe (Haccer& h, MyFloat& it) {
        float f = it;  // Too encapsulated to get a reference.
        h.as_float(f);
        it = MyFloat(f);
    }
};

template <class C>
struct MyWrapper {
    C val;
};
template <class C>
bool operator == (MyWrapper<C> a, MyWrapper<C> b) { return a.val==b.val; }

template <class C> struct Haccable<MyWrapper<C>> : Haccability<MyWrapper<C>> {
    void describe (Haccer& h, MyWrapper<C>& it) {
        run_description(h, it.val);  // manual delegation
    }
};


MyWrapper<int> wi {0};

#include "../../tap/inc/tap.h"
tap::Tester haccable_tester ("haccable", [](){
    using namespace hacc;
    using namespace tap;
    plan(15);
    is(hacc_from((int)4).get_integer(), 4, "hacc_from<int> works");
    is(hacc_to<int>(Hacc(35)), 35, "hacc_to<int> works");
    doesnt_throw([](){ wi = from_hacc<MyWrapper<int>>(Hacc(34)); }, "from_hacc on a template haccable");
    is(wi.val, 34, "...works");
    doesnt_throw([](){ update_from_hacc(wi, Hacc(52)); }, "update_from_hacc on a template haccable");
    is(wi.val, 52, "...and it works");
    Hacc* ahcs [2] = {new Hacc(34.4), new Hacc(52.123)};
    is(from_hacc<Vectorly>(Hacc(Array(ahcs, ahcs+2))), Vectorly{34.4, 52.123}, "Vectorly accepts Array");
    is(hacctype<float>(), String("float"), "hacctype declaration and query from cpptype works");
    ok(HaccTable::by_hacctype("float"), "Can get HaccTables by primary hacctype");
    ok(HaccTable::by_hacctype("Float"), "Can get HaccTables by secondary hacctype");
    is(HaccTable::by_hacctype("FLOAT"), null, "HaccTable::by_hacctype with a non-existant hacctype returns null");
    is(haccid(vy1), String("vy1"), "haccid");
    is(haccid(vy2), String("vy2"), "haccid");
    is(find_by_haccid<Vectorly>("vy1"), &vy1, "find_by_haccid");
    is(find_by_id<Vectorly>("vy2"), &vy2, "find_by_id is the same");
});

