#include "../inc/tree.h"
#include "../inc/files.h"

#include "../../tap/inc/tap.h"
tap::Tester hacc_tester ("hacc/tree", [](){
    using namespace hacc;
    using namespace tap;
    plan(11);
    is(Tree().form(), NULLFORM, "nullary constructor makes null hacc");
    is(Tree(true).as<bool>(), true, "bool roundtrip");
    is(Tree(false).as<bool>(), false, "false roundtrip");
    is(Tree(45).as<int>(), 45, "integer roundtrip");
    is(Tree(-45).as<int>(), -45, "negative integer roundtrip");
    is(Tree(32.f).as<float>(), 32.f, "float roundtrip");
    is(Tree(32.0).as<double>(), 32.0, "double roundtrip");
    is(Tree("hello").as<String>(), String("hello"), "string roundtrip");
    is(Tree(Path("asdf")).as<Path>(), Path("asdf"), "reference roundtrip");
    is(Tree(Array()).as<const Array&>(), Array(), "Empty array roundtrip");
    const Array& a = Array{Tree(3), Tree(4.f)};
    is(Tree(a).as<const Array&>(), a, "Full array roundtrip");
});
