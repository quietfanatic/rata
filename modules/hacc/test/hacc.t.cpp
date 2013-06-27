#include "../inc/hacc.h"

#include "../../tap/inc/tap.h"
tap::Tester hacc_tester ("hacc", [](){
    using namespace hacc;
    using namespace tap;
    plan(11);
    is(Hacc().form, NULLFORM, "nullary constructor makes null hacc");
    is(Hacc(true).b, true, "bool roundtrip");
    is(Hacc(false).b, false, "false roundtrip");
    is(Hacc(45).i, 45, "integer roundtrip");
    is(Hacc(-45).i, -45, "negative integer roundtrip");
    is(Hacc(32.f).f, 32.f, "float roundtrip");
    is(Hacc(32.0).d, 32.0, "double roundtrip");
    is(Hacc(String("hello")).s, std::string("hello"), "string roundtrip");
    is(*Hacc(new Path("asdf")).p, Path("asdf"), "reference roundtrip");
    is(*Hacc(Array()).a, Array(), "Empty array roundtrip");
    const Array& a = Array{new Hacc (3), new Hacc (4.f)};
    is(*Hacc(a).a, a, "Full array roundtrip");
});
