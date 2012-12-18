
#include "../inc/haccable_standard.h"


#include "../../tap/inc/tap.h"
tap::Tester haccable_standard_tester ("haccable_standard", [](){
    using namespace hacc;
    using namespace tap;
    plan(28);
#define TEST_STD(type, value, valtype) \
    is((type(*)())([](){return (type)hacc_from<type>((type)value).get_##valtype();}), (type)value, "standard hacc_from on " #type); \
    is((type(*)())([](){return (type)hacc_to<type>(Hacc((type)value));}), (type)value, "standard hacc_to on " #type);
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
    TEST_STD(std::string, "asdf", string);
});

