
#include "../inc/haccable.h"

using namespace hacc;

HCB_BEGIN(int)
    to([](const int& x){ return new_hacc(x); });
    update_from([](int& x, const Hacc* h){ x = h->get_integer(); });
HCB_END(int)

HCB_BEGIN(float)
    to([](const float& x){ return new_hacc(x); });
    update_from([](float& x, const Hacc* h){ x = h->get_float(); });
HCB_END(float)

struct Vectorly {
    float x;
    float y;
    bool operator == (Vectorly o) const { return x==o.x && y==o.y; }
};

Vectorly vy1 { 1, 2 };
Vectorly vy2 { 3, 4 };

HCB_BEGIN(Vectorly)
    get_id([](const Vectorly& vy){
        if (&vy == &vy1) return "vy1";
        else if (&vy == &vy2) return "vy2";
        else return "";
    });
    find_by_id([](String id){
        if (id == "vy1") return &vy1;
        else if (id == "vy2") return &vy2;
        else return (Vectorly*)null;
    });
    attr("x", member(&Vectorly::x));
    attr("y", member(&Vectorly::y));
    elem(member(&Vectorly::x));
    elem(member(&Vectorly::y));
HCB_END(Vectorly)

struct MyFloat {
    float val;
    operator float () const { return val; }
    MyFloat (float val) : val(val) { }
    MyFloat () { }
    bool operator == (MyFloat o) const { return val==o.val; }
};

HCB_BEGIN(MyFloat)
    delegate(member(&MyFloat::val));
HCB_END(MyFloat)

template <class C>
struct MyWrapper {
    C val;
};
template <class C>
bool operator == (MyWrapper<C> a, MyWrapper<C> b) { return a.val==b.val; }

HCB_TEMPLATE_BEGIN(<class C>, MyWrapper<C>)
    delegate(member(&MyWrapper<C>::val));
HCB_TEMPLATE_END(<class C>, MyWrapper<C>)


MyWrapper<int> wi {0};

#include "../../tap/inc/tap.h"
tap::Tester haccable_tester ("haccable", [](){
    using namespace hacc;
    using namespace tap;
    plan(13);
    is(hacc_from((int)4)->get_integer(), 4, "hacc_from<int> works");
    is(hacc_to_value<int>(new_hacc(35)), 35, "hacc_to<int> works");
    doesnt_throw([](){ wi = value_from_hacc<MyWrapper<int>>(new_hacc(34)); }, "from_hacc on a template haccable");
    is(wi.val, 34, "...works");
    doesnt_throw([](){ update_from_hacc(wi, new_hacc(52)); }, "update_from_hacc on a template haccable");
    is(wi.val, 52, "...and it works");
    is(value_from_hacc<Vectorly>(new_hacc({new_hacc(34.0), new_hacc(52.0)})), Vectorly{34.0, 52.0}, "Vectorly accepts Array");
    is(value_from_hacc<Vectorly>(new_hacc({std::pair<String, const Hacc*>("x", new_hacc(32.0)), std::pair<String, const Hacc*>("y", new_hacc(54.0))})), Vectorly{32.0, 54.0}, "Vectorly accepts Object");
    is(to_hacc(Vectorly{2.0, 4.0})->form(), OBJECT, "Vectorly turns into Object by default");
    is(to_hacc(Vectorly{2.0, 4.0})->as_object()->attr("y")->as_float()->f, 4.f, "Vectorly Object has atribute 'y'");
    is(get_id(vy1), String("vy1"), "get_id");
    is(get_id(vy2), String("vy2"), "get_id");
    is(find_by_id<Vectorly>("vy1"), &vy1, "find_by_id");
});

