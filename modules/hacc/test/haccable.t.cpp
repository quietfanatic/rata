
#include "../inc/haccable.h"

HACCABLE(int, {
    d::type("int, yo!");
    d::to([](const int& i) {
        return hacc::Hacc(i);
    });
    d::from([](hacc::Hacc h) {
        return (int)h.get_integer();
    });
})

struct MyThing {
    int x;
    int y;
    bool operator == (MyThing o) const { return x==o.x && y==o.y; }
};
HACCABLE(MyThing, {
    d::type("MyThing");
    d::to([](const MyThing& t){
        hacc::Array a (2);
        a[0] = hacc::to_hacc<int>(t.x);
        a[1] = hacc::to_hacc<int>(t.y);
        return hacc::Hacc(a);
    });
    d::from([](hacc::Hacc h) {
        auto a = h.get_array();
        return MyThing{ hacc::from_hacc<int>(a[0]), hacc::from_hacc<int>(a[1]) };
    });
})

struct MyFloat {
    float val;
    operator float () const { return val; }
    MyFloat (float val) : val(val) { }
    MyFloat () { }
    bool operator == (MyFloat o) const { return val==o.val; }
};
HACCABLE(MyFloat, { d::like_float(); })

template <class C>
struct MyWrapper {
    C val;
};
template <class C>
bool operator == (MyWrapper<C> a, MyWrapper<C> b) { return a.val==b.val; }
HACCABLE_TEMPLATE(<class C>, MyWrapper<C>, {
    d::get_type([](const MyWrapper<C>& v){
        return "MyWrapper<" + hacc::get_type(v.val) + ">";
    });
    d::to([](const MyWrapper<C>& v){ return hacc::to_hacc(v.val); });
    d::from([](hacc::Hacc h){ return MyWrapper<C>{hacc::from_hacc<C>(h)}; });
})
 // Let's try it without an explicit instantiation.


#include "../../tap/inc/tap.h"
tap::Tester haccable_tester ("haccable", [](){
    using namespace hacc;
    using namespace tap;
    plan(13);
    diag("Using custom Haccable<int> with 'to' and 'from'");
    is(hacc_from((int)4).get_integer(), 4, "hacc_from<int> works");
    is(string_from((int)552), "552", "string_from<int> works");
    is(hacc_to<int>(Hacc(35)), 35, "hacc_to<int> works");
    is(string_to<int>("-789"), -789, "string_to<int> works");
    is(get_type<int>(), "int, yo!", "get_type<int> works");
    is(get_type((int)443), "int, yo!", "get_type works on instance of int");
    is(string_from(MyThing{324, 425}), "[324, 425]", "string_from<MyThing> works");
    is(string_to<MyThing>("[-51, 20]"), MyThing{-51, 20}, "string_to<MyThing> works");
    is(string_from(MyFloat(2.0)), "2~40000000", "like_float() defines a to");
    is(string_to<MyFloat>("32.0"), MyFloat(32.0), "like_float() defines a from");
    is(string_from(MyWrapper<int>{54}), "54", "to in a HACCABLE_TEMPLATE works");
    is(string_to<MyWrapper<int>>("192"), MyWrapper<int>{192}, "from in a HACCABLE_TEMPLATE works");
    is(get_type(MyWrapper<int>{54}), "MyWrapper<int, yo!>", "get_type in a HACCABLE_TEMPLATE works");
});

