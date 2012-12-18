
#include "../inc/haccable.h"

using namespace hacc;

template <> struct Haccable<int> : Haccability<int> {
    void describe (Haccer& h, int& it) {
        h.as_integer(it);
    }
};
template <> struct Haccable<float> : Haccability<float> {
    void describe (Haccer& h, float& it) {
        h.as_float(it);
    }
};

struct Vectorly {
    float x;
    float y;
    bool operator == (Vectorly o) const { return x==o.x && y==o.y; }
};

template <> struct Haccable<Vectorly> : Haccability<Vectorly> {
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
    plan(7);
    is(hacc_from((int)4).get_integer(), 4, "hacc_from<int> works");
    is(hacc_to<int>(Hacc(35)), 35, "hacc_to<int> works");
    doesnt_throw([](){ wi = from_hacc<MyWrapper<int>>(Hacc(34)); }, "from_hacc on a template haccable");
    is(wi.val, 34, "...works");
    doesnt_throw([](){ update_from_hacc(wi, Hacc(52)); }, "update_from_hacc on a template haccable");
    is(wi.val, 52, "...and it works");
    Hacc* ahcs [2] = {new Hacc(34.4), new Hacc(52.123)};
    is(from_hacc<Vectorly>(Hacc(Array(ahcs, ahcs+2))), Vectorly{34.4, 52.123}, "Vectorly accepts Array");
});

