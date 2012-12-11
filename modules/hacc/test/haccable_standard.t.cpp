
#include "../inc/haccable_standard.h"


#include "../../tap/inc/tap.h"
tap::Tester haccable_standard_tester ("haccable_standard", [](){
    using namespace hacc;
    using namespace tap;
    plan(29);
#define TEST_CONVERTIBLE(type, value, valtype) \
    is((type)hacc_from((type)value).get_##valtype(), (type)value, "standard hacc_from on " #type); \
    is((type)hacc_to<type>(Hacc((type)value)), (type)value, "standard hacc_to on " #type);
    TEST_CONVERTIBLE(Null, null, null);
    TEST_CONVERTIBLE(bool, true, bool);
    TEST_CONVERTIBLE(char, 'c', integer);
    TEST_CONVERTIBLE(int8, -5, integer);
    TEST_CONVERTIBLE(uint8, 6, integer);
    TEST_CONVERTIBLE(int16, -7, integer);
    TEST_CONVERTIBLE(uint16, 8, integer);
    TEST_CONVERTIBLE(int32, -9, integer);
    TEST_CONVERTIBLE(uint32, 10, integer);
    TEST_CONVERTIBLE(int64, -11, integer);
    TEST_CONVERTIBLE(uint64, 12, integer);
    TEST_CONVERTIBLE(float, 2.0, float);
    TEST_CONVERTIBLE(double, -2.0, double);
    TEST_CONVERTIBLE(std::string, "asdf", string);
    float f = 32.0;
    is(hacc_to<float*>(hacc_from(&f)), &f, "round-trip on float*");
});

