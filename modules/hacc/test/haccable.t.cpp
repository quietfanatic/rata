
#include "../inc/haccable.h"
#include "../inc/haccable_standard.h"
using namespace hacc;

struct Vectorly {
    float x;
    float y;
    bool operator == (Vectorly o) const { return x==o.x && y==o.y; }
};

Vectorly vy1 { 1, 2 };
Vectorly vy2 { 3, 4 };

HCB_BEGIN(Vectorly*)
    to_tree([](Vectorly* const& vy){
        if (vy == &vy1) return Tree("vy1");
        else if (vy == &vy2) return Tree("vy2");
        else return Tree(null);
    });
    fill([](Vectorly*& vy, Tree t){
        if (t.form() == STRING) {
            std::string id = t.as<String>();
            if (id == "vy1") vy = &vy1;
            else if (id == "vy2") vy = &vy2;
            else vy = null;
        }
        else throw X::Error("Expected string hacc but got " + form_name(t.form()) + " hacc");
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
    keys(value_funcs<std::vector<String>>([](const MyUnion& u)->std::vector<String>{
        switch (u.type) {
            case MyUnion::INT: return std::vector<String>(1, "i");
            case MyUnion::FLOAT: return std::vector<String>(1, "f");
            default: return std::vector<String>(1, "n");
        }
    }, [](MyUnion& u, std::vector<String> ks)->void{
        if (ks.size() != 1) throw X::Error("A MyUnion in object form must have one key");
        else if (ks[0] == "i") u.type = MyUnion::INT;
        else if (ks[0] == "f") u.type = MyUnion::FLOAT;
        else if (ks[0] == "n") u.type = MyUnion::NONE;
        else throw X::Error("Unknown MyUnion variant: " + ks[0]);
    }));
    attr("i", value_methods(&MyUnion::get_i, &MyUnion::set_i));
    attr("f", value_methods(&MyUnion::get_f, &MyUnion::set_f));
HCB_END(MyUnion)

enum MyEnum {
    VALUE1,
    VALUE2,
    VALUE3
};
HCB_BEGIN(MyEnum)
    name("MyEnum");
    value("value1", VALUE1);
    value("value2", VALUE2);
    value("value3", VALUE3);
HCB_END(MyEnum)

struct MyNamed {
    std::string name;
    MyNamed (std::string name) : name(name) { }
};
MyNamed mn1 ("asdf");
MyNamed mn2 ("fdsa");

std::vector<MyNamed*> mns {&mn1, &mn2};

HCB_BEGIN(MyNamed*)
    name("MyNamed*");
    hacc_pointer_by_member(&MyNamed::name, mns, true);
HCB_END(MyNamed*)

template <class C>
Tree to_tree (C* p) { return Reference(p).to_tree(); }
template <class C>
void from_tree (C* p, Tree t) { Reference(p).from_tree(t); }

int32 i = 4;
Vectorly vy;
Vectorly* vyp;
MyUnion mu;
MyEnum me = VALUE2;
Dynamic dyn = Dynamic::New<int32>(3);
MyNamed* mnp = mns[0];

#include "../../tap/inc/tap.h"
tap::Tester haccable_tester ("hacc/haccable", [](){
    using namespace hacc;
    using namespace tap;
    plan(39);
    is(to_tree(&i).as<int>(), 4, "to_tree on int32 works");
    doesnt_throw([](){ from_tree(&i, Tree(35)); }, "from_tree on int32");
    is(i, 35, "...works");
    doesnt_throw([](){ from_tree(&wi, Tree(34)); }, "from_tree on a template haccable");
    is(wi.val, 34, "...works");
    doesnt_throw([](){ from_tree(&vy, Tree(Array{Tree(34.0), Tree(52.0)})); }, "from_tree using elems");
    is(vy, Vectorly{34.0, 52.0}, "...works");
    doesnt_throw([](){ from_tree(&vy, Tree(Object{Pair("x", Tree(32.0)), Pair("y", Tree(54.0))})); }, "from_tree using attrs");
    is(vy, Vectorly{32.0, 54.0}, "...works");
    vy = Vectorly{2.0, 4.0};
    is(to_tree(&vy).form(), OBJECT, "Vectorly turns into Object by default");
    is(to_tree(&vy).as<const Object&>().at(1).first, String("y"), "Vectorly Object has atribute 'y'");
    is(to_tree(&vy).attr("y").as<float>(), 4.f, "And its value is correct");
    vyp = &vy1;
    is(to_tree(&vyp).form(), STRING, "Custom string <- pointer");
    is(to_tree(&vyp).as<String>(), String("vy1"), "Custom string <- pointer");
    vyp = &vy2;
    is(to_tree(&vyp).form(), STRING, "Custom string <- pointer");
    is(to_tree(&vyp).as<String>(), String("vy2"), "Custom string <- pointer");
    doesnt_throw([](){ from_tree(&vyp, Tree("vy1")); }, "Custom string -> pointer behavior");
    is(vyp, &vy1, "...works");
    doesnt_throw([](){ from_tree(&mu, Tree(Object{Pair("i", Tree(35))})); }, "Union using attrs as variants");
    is(mu.i.i, 35, "...can be read from hacc");
    doesnt_throw([](){ from_tree(&mu, Tree(Object{Pair("f", Tree(31.2f))})); }, "Union using attrs as variants");
    is(mu.f.f, 31.2f, "...can be read from hacc");
    mu = MyUnion(71);
    is(to_tree(&mu).form(), OBJECT, "Union is written as object");
    is(to_tree(&mu).attr("i").as<int>(), 71, "Union can be written to hacc");
    mu = MyUnion(4.f);
    is(to_tree(&mu).attr("f").as<float>(), 4.f, "Union can be written to hacc");
    is(to_tree(&dyn).form(), OBJECT, "Dynamic is written as object");
    is(to_tree(&dyn).as<const Object&>().at(0).first, String("int32"), "Dynamic has type as key");
    is(to_tree(&dyn).attr("int32").as<int>(), 3, "Dynamic has value as value");
    doesnt_throw([](){ from_tree(&dyn, Tree(Object{Pair("float", Tree(99.7f))})); }, "from_tree on Dynamic");
    is(dyn.type, Type(Type::CppType<float>()), "...sets the right type");
    is(*(float*)dyn.addr, 99.7f, "...and sets the right value");
    is(to_tree(&me).form(), STRING, "Enumish types written as string");
    is(to_tree(&me).as<String>(), String("value2"), "Enumish types use correct value when writing");
    doesnt_throw([](){ from_tree(&me, Tree("value3")); }, "from_tree on enumish type");
    is(me, VALUE3, "...sets the right type");
    is(to_tree(&mnp).form(), STRING, "hacc_pointer_by_member writes as right type");
    is(to_tree(&mnp).as<String>(), String("asdf"), "hacc_pointer_by_member writes correct value");
    doesnt_throw([](){ from_tree(&mnp, Tree("fdsa")); }, "hacc_pointer_by_member can do from_tree");
    is(mnp, mns[1], "hacc_pointer_by_member sets correct value");
});

