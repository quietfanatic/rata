
#include "../inc/haccable.h"
using namespace hacc;

HCB_BEGIN(int32)
    type_name("int32");
    to([](const int32& x){ return new_hacc(x); });
    update_from([](int32& x, Hacc* h){ x = h->get_integer(); });
HCB_END(int32)

HCB_BEGIN(float)
    to([](const float& x){ return new_hacc(x); });
    update_from([](float& x, Hacc* h){ x = h->get_float(); });
HCB_END(float)

struct Vectorly {
    float x;
    float y;
    bool operator == (Vectorly o) const { return x==o.x && y==o.y; }
};

Vectorly vy1 { 1, 2 };
Vectorly vy2 { 3, 4 };

HCB_BEGIN(Vectorly*)
    to([](Vectorly* const& vy){
        if (vy == &vy1) return new_hacc(String("vy1"));
        else if (vy == &vy2) return new_hacc(String("vy2"));
        else return new_hacc(null);
    });
    update_from([](Vectorly*& vy, Hacc* h){
        std::string id = h->get_string();
        if (id == "vy1") vy = &vy1;
        else if (id == "vy2") vy = &vy2;
        else vy = null;
    });
HCB_END(Vectorly*)
HCB_BEGIN(Vectorly)
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

MyWrapper<int32> wi {0};

union MyUnion {
    enum Type {
        NONE,
        INT,
        FLOAT,
    } type;
    struct I {
        Type type;
        int32 i;
    } i;
    struct F {
        Type type;
        float f;
    } f;
    MyUnion () : type{NONE} { }
    MyUnion (int32 i) : i{INT, i} { }
    MyUnion (float f) : f{FLOAT, f} { }
    void set_i (int32 i_) { type = INT; i.i = i_; }
    void set_f (float f_) { type = FLOAT; f.f = f_; }
    int32 get_i () const { return i.i; }
    float get_f () const { return f.f; }
};

HCB_BEGIN(MyUnion)
    variant("i", value_methods(&MyUnion::get_i, &MyUnion::set_i));
    variant("f", value_methods(&MyUnion::get_f, &MyUnion::set_f));
    select_variant([](const MyUnion& u)->String{
        switch (u.type) {
            case MyUnion::INT: return "i";
            case MyUnion::FLOAT: return "f";
            default: return "";
        }
    });
HCB_END(MyUnion)


#include "../../tap/inc/tap.h"
tap::Tester haccable_tester ("haccable", [](){
    using namespace hacc;
    using namespace tap;
    plan(20);
    is(hacc_from((int32)4)->get_integer(), 4, "hacc_from<int32> works");
    is(hacc_to_value<int32>(new_hacc(35)), 35, "hacc_to<int32> works");
    doesnt_throw([](){ wi = value_from_hacc<MyWrapper<int32>>(new_hacc(34)); }, "from_hacc on a template haccable");
    is(wi.val, 34, "...works");
    doesnt_throw([](){ update_from_hacc(wi, new_hacc(52)); }, "update_from_hacc on a template haccable");
    is(wi.val, 52, "...and it works");
    is(value_from_hacc<Vectorly>(new_hacc({new_hacc(34.0), new_hacc(52.0)})), Vectorly{34.0, 52.0}, "Vectorly accepts Array");
    is(value_from_hacc<Vectorly>(new_hacc({new_attr("x", 32.0), new_attr("y", 54.0)})), Vectorly{32.0, 54.0}, "Vectorly accepts Object");
    is(to_hacc(Vectorly{2.0, 4.0})->form(), OBJECT, "Vectorly turns into Object by default");
    is(to_hacc(Vectorly{2.0, 4.0})->as_object()->attr("y")->as_float()->f, 4.f, "Vectorly Object has atribute 'y'");
    is(to_hacc(&vy1)->get_string(), String("vy1"), "string <- pointer");
    is(to_hacc(&vy2)->get_string(), String("vy2"), "string <- pointer");
    is(value_from_hacc<Vectorly*>(new_hacc(String("vy1"))), &vy1, "string -> pointer");
    is(value_from_hacc<MyUnion>(new_hacc({new_attr("i", 35)})).i.i, 35, "Union with declared variants can be read from hacc");
    is(value_from_hacc<MyUnion>(new_hacc({new_attr("f", 32.f)})).f.f, 32.f, "Union with declared variants can be read from hacc");
    is(hacc_from(MyUnion(71))->form(), OBJECT, "Union with declared variants is written as object");
    is(hacc_from(MyUnion(71))->as_object()->name_at(0), "i", "Union with declared variants can be written to hacc");
    is(hacc_from(MyUnion(71))->as_object()->value_at(0)->get_integer(), 71, "Union with declared variants can be written to hacc");
    is(hacc_from(MyUnion(4.f))->as_object()->name_at(0), "f", "Union with declared variants can be written to hacc");
    is(hacc_from(MyUnion(4.f))->as_object()->value_at(0)->get_float(), 4.f, "Union with declared variants can be written to hacc");
});

