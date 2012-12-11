
#include "../inc/haccable.h"

struct MyThing {
    int x;
    int y;
};

HACCABLE(int, {
    type("int");
    to([](const int& i) {
        return hacc::Hacc(i);
    });
    from([](hacc::Hacc h) {
        return (int)h.get_integer();
    });
})

HACCABLE(MyThing, {
    to([](const MyThing& t){
        hacc::Array a (2);
        a[0] = hacc::to_hacc<int>(t.x);
        a[1] = hacc::to_hacc<int>(t.y);
        return hacc::Hacc(a);
    });
    from([](hacc::Hacc h) {
        auto a = h.get_array();
        return MyThing { hacc::from_hacc<int>(a[0]), hacc::from_hacc<int>(a[1]) };
    });
})

#include "../../tap/inc/tap.h"
tap::Tester haccable_tester ("haccable", [](){
    using namespace hacc;
    using namespace tap;
    plan(4);
    diag("Using custom Haccable<int> with 'to' and 'from'");
    is(hacc_from((int)4).get_integer(), 4, "hacc_from<int> works.");
    is(string_from((int)552), "552", "string_from<int> works.");
    is(hacc_to<int>(Hacc(35)), 35, "hacc_to<int> works.");
    is(string_to<int>("-789"), -789, "string_to<int> works.");
});

