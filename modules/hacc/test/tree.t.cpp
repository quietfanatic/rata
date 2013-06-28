#include "../inc/tree.h"
#include "../inc/files.h"

#include "../../tap/inc/tap.h"
tap::Tester hacc_tester ("hacc/tree", [](){
    using namespace hacc;
    using namespace tap;
    plan(11);
    is(Tree().form, NULLFORM, "nullary constructor makes null hacc");
    is(Tree(true).b, true, "bool roundtrip");
    is(Tree(false).b, false, "false roundtrip");
    is(Tree(45).i, 45, "integer roundtrip");
    is(Tree(-45).i, -45, "negative integer roundtrip");
    is(Tree(32.f).f, 32.f, "float roundtrip");
    is(Tree(32.0).d, 32.0, "double roundtrip");
    is(Tree(String("hello")).s, String("hello"), "string roundtrip");
    is(*Tree(new Path("asdf")).p, Path("asdf"), "reference roundtrip");
    is(*Tree(Array()).a, Array(), "Empty array roundtrip");
    const Array& a = Array{new Tree (3), new Tree (4.f)};
    is(*Tree(a).a, a, "Full array roundtrip");
});
