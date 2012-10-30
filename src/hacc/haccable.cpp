
#include "haccable.h"

#ifndef DISABLE_TESTS
#include "tap.h"

tap::Tester haccable_tester ("haccable", [](){
    using namespace hacc;
    using namespace tap;
    plan(39);

#define HACCABLE_TEST_ROUNDTRIP(t, value) \
    { \
        is(from_hacc<t>(to_hacc<t>(t(value))), t(value), #t " to and from Hacc"); \
        t x; \
        update_from_hacc<t>(x, to_hacc<t>(t(value))); \
        is(x, t(value), #t " to and update from Hacc"); \
        is(*new_from_hacc<t>(to_hacc<t>(t(value))), t(value), #t " to and new from Hacc"); \
    }
    HACCABLE_TEST_ROUNDTRIP(char8, 32)
    HACCABLE_TEST_ROUNDTRIP(int8, -54)
    HACCABLE_TEST_ROUNDTRIP(uint8, 132)
    HACCABLE_TEST_ROUNDTRIP(int16, -1234)
    HACCABLE_TEST_ROUNDTRIP(uint16, 53154)
    HACCABLE_TEST_ROUNDTRIP(int32, -2000000000)
    HACCABLE_TEST_ROUNDTRIP(uint32, 4000000000)
    HACCABLE_TEST_ROUNDTRIP(int64, -1000000000000)
    HACCABLE_TEST_ROUNDTRIP(uint64, 1000000000000)
    HACCABLE_TEST_ROUNDTRIP(float, 3.14159)
    HACCABLE_TEST_ROUNDTRIP(double, 3.1415926535)
    HACCABLE_TEST_ROUNDTRIP(std::string, "asdf")
    HACCABLE_TEST_ROUNDTRIP(std::string, "\n\\\"")
});

#endif

