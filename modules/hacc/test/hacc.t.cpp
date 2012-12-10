#include "../inc/hacc.h"

#include "../../tap/inc/tap.h"
tap::Tester hacc_tester ("hacc", [](){
    using namespace hacc;
    using namespace tap;
    plan(14);
    is(Hacc(null).get_null(), null, "null roundtrip");
    is(Hacc().get_null(), null, "Default hacc is null");
    is(Hacc(true).get_bool(), true, "true roundtrip");
    is(Hacc(false).get_bool(), false, "false roundtrip");
    is(Hacc(45).get_integer(), 45, "integer roundtrip");
    is(Hacc(-45).get_integer(), -45, "negative integer roundtrip");
    is(Hacc(32.f).get_float(), 32.f, "float roundtrip");
    is(Hacc(32.F).get_double(), 32.F, "double roundtrip");
    is(Hacc(32.f).get_double(), 32.F, "float -> double");
    is(Hacc(32.F).get_float(), 32.f, "double -> float");
    is(Hacc(32).get_float(), 32.f, "integer -> float");
    is(Hacc(32).get_double(), 32.F, "integer -> double");
    is(Hacc(String("hello")).get_string(), std::string("hello"), "string roundtrip");
    is(Hacc(Array()).get_array(), Array(), "Empty array roundtrip");
});



