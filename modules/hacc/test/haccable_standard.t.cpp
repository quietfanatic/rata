
#include "../inc/haccable_standard.h"


struct IPp {
    int i;
    IPp* p;
};

HCB_BEGIN(IPp)
    elem(member(&IPp::i));
    elem(member(&IPp::p));
HCB_END(IPp)
HCB_BEGIN(IPp*)
    pointer(supertype<IPp*>());
HCB_END(IPp*)

const hacc::Hacc* h2;
std::vector<IPp> v2;


#include "../../tap/inc/tap.h"
tap::Tester haccable_standard_tester ("haccable_standard", [](){
    using namespace hacc;
    using namespace tap;
    plan(30);
#define TEST_STD(type, value, form) \
    is((type(*)())([](){return (type)hacc_from<type>((type)value)->get_##form();}), (type)value, "standard hacc_from on " #type); \
    is((type(*)())([](){return (type)hacc_to_value<type>(new_hacc((type)value));}), (type)value, "standard hacc_to on " #type);
    TEST_STD(Null, null, null);
    TEST_STD(bool, true, bool);
    TEST_STD(char, 'c', integer);
    TEST_STD(int8, -5, integer);
    TEST_STD(uint8, 6, integer);
    TEST_STD(int16, -7, integer);
    TEST_STD(uint16, 8, integer);
    TEST_STD(int32, -9, integer);
    TEST_STD(uint32, 10, integer);
    TEST_STD(int64, -11, integer);
    TEST_STD(uint64, 12, integer);
    TEST_STD(float, 2.0, float);
    TEST_STD(double, -2.0, double);
    std::vector<int> v {1, 2, 3, 4, 5};
    is(hacc_from(v)->as_array()->elem(2)->get_integer(), 3, "Hacc from std::vector seems to work");
    const Hacc* h = new_hacc({new_hacc(3), new_hacc(4), new_hacc(5)});
    is(hacc_to_value<std::vector<int>>(h)[1], 4, "Hacc to std::vector seems to work");
    h2 = new_hacc({
        new_hacc({new_hacc(4), new_hacc(Ref("thing2"))}, "thing1"),
        new_hacc({new_hacc(3), new_hacc(Ref("thing1"))}, "thing2")
    });
    doesnt_throw([](){ update_from_hacc(v2, h2); }, "ID wrangling within a std::vector doesn't throw");
    is(v2[0].p, &v2[1], "ID wrangling within a std::vector gives the right answer");
    is(v2[1].p, &v2[0], "ID wrangling within a std::vector gives the right answer");
});

