
#include "../inc/haccable_standard.h"


#include "../../tap/inc/tap.h"
tap::Tester haccable_standard_tester ("haccable_standard", [](){
    using namespace hacc;
    using namespace tap;
    plan(26);
#define TEST_STD(type, value, form) \
    is((type(*)())([](){return (type)hacc_from<type>((type)value)->get_##form();}), (type)value, "standard hacc_from on " #type); \
    is((type(*)())([](){return (type)hacc_to<type>(new_hacc((type)value));}), (type)value, "standard hacc_to on " #type);
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
//    std::vector<int> v {1, 2, 3, 4, 5};
//    is(hacc_from(v).get_array()[2]->get_integer(), 3, "Hacc from std::vector seems to work");
//    Array a;
//    a.push_back(std::move(std::unique_ptr<Hacc>(new Hacc(3))));
//    a.push_back(std::move(std::unique_ptr<Hacc>(new Hacc(4))));
//    a.push_back(std::move(std::unique_ptr<Hacc>(new Hacc(5))));
//    Hacc h (std::move(a));
//    is(hacc_to<std::vector<int>>(h)[1], 4, "Hacc to std::vector seems to work");
});

