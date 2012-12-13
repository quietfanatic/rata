
#include "../inc/haccable.h"

HACCABLE(int, {
    d.hacctype("int, yo!");
    d.to_hacc([](const int& i) {
        return hacc::Hacc(i);
    });
    d.from_hacc([](hacc::Hacc h) {
        return (int)h.get_integer();
    });
})

struct MyThing {
    int x;
    int y;
    bool operator == (MyThing o) const { return x==o.x && y==o.y; }
};
HACCABLE(MyThing, {
    using namespace hacc;
    d.hacctype("MyThing");
    d.to_hacc([](const MyThing& t){
        hacc::Array a (2);
        a[0] = to_hacc<int>(t.x);
        a[1] = to_hacc<int>(t.y);
        return Hacc(a);
    });
    d.from_hacc([](Hacc h) {
        auto a = h.get_array();
        return MyThing{ from_hacc<int>(a[0]), from_hacc<int>(a[1]) };
    });
})

struct MyFloat {
    float val;
    operator float () const { return val; }
    MyFloat (float val) : val(val) { }
    MyFloat () { }
    bool operator == (MyFloat o) const { return val==o.val; }
};
HACCABLE(MyFloat, { d.like_float(); })

template <class C>
struct MyWrapper {
    C val;
};
template <class C>
bool operator == (MyWrapper<C> a, MyWrapper<C> b) { return a.val==b.val; }
HACCABLE_TEMPLATE(<class C>, MyWrapper<C>, {
    d.hacctype([](){
        return "MyWrapper<" + hacc::hacctype<C>() + ">";
    });
    d.to_hacc([](const MyWrapper<C>& v){ return hacc::to_hacc(v.val); });
    d.from_hacc([](hacc::Hacc h){ return MyWrapper<C>{hacc::from_hacc<C>(h)}; });
})
 // Let's try it without an explicit instantiation.

struct NoConstructor {
    int x;
    NoConstructor (int x) : x(x) { };
};
HACCABLE(NoConstructor, {
    d.update_from_hacc([](NoConstructor& v, hacc::Hacc h){
        v.x = h.get_integer();
    });
})

struct ID_Tester {
    int x;
} id_tester {43};
HACCABLE(ID_Tester, {
    d.haccid([](const ID_Tester& v){ return hacc::String("The only!"); });
    d.find_by_haccid([](hacc::String s){
        if (s == "The only!") return &id_tester;
        else return (ID_Tester*)hacc::null;
    });
})

#include "../../tap/inc/tap.h"
tap::Tester haccable_tester ("haccable", [](){
    using namespace hacc;
    using namespace tap;
    plan(9);
    diag("Using custom Haccable<int> with 'to' and 'from'");
    is(hacc_from((int)4).get_integer(), 4, "hacc_from<int> works");
//    is(string_from((int)552), "552", "string_from<int> works");
    is(hacc_to<int>(Hacc(35)), 35, "hacc_to<int> works");
//    is(string_to<int>("-789"), -789, "string_to<int> works");
    is(hacctype<int>(), "int, yo!", "hacctype<int> works");
    is(hacctype((int)443), "int, yo!", "hacctype works on instance of int");
//    is(string_from(MyThing{324, 425}), "[324, 425]", "string_from<MyThing> works");
//    is(string_to<MyThing>("[-51, 20]"), MyThing{-51, 20}, "string_to<MyThing> works");
//    is(string_from(MyFloat(2.0)), "2~40000000", "like_float() defines a to");
//    is(string_to<MyFloat>("32.0"), MyFloat(32.0), "like_float() defines a from");
//    is(string_from(MyWrapper<int>{54}), "54", "to in a HACCABLE_TEMPLATE works");
//    is(string_to<MyWrapper<int>>("192"), MyWrapper<int>{192}, "from in a HACCABLE_TEMPLATE works");
    is(hacctype<MyWrapper<int>>(), "MyWrapper<int, yo!>", "hacctype in a HACCABLE_TEMPLATE works");
    throws<hacc::Error>([](){ from_hacc<NoConstructor>(Hacc(42)); }, "Throw runtime exception if nullary constructor is required but unavailable.");
    is(haccid(id_tester), "The only!", "haccid appears to work");
    is(find_by_haccid<ID_Tester>(String("The only!"))->x, 43, "find_by_haccid appears to work");
    int x = 32;
    is(find_by_haccid<int>(haccid(x)), &x, "Default haccid and find_by_haccid appear to work, at least together");
});

