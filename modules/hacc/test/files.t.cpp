#include "../inc/files.h"

 // TODO: right now we're being naughty and relying on
 //  haccable.t.cpp having been linked in with this file

using namespace hacc;

#include "../../tap/inc/tap.h"
tap::Tester files_tester ("hacc/files", [](){
    using namespace tap;
    plan(5);
    ok(!File("../test/seven.hacc").loaded(), "File is not loaded before load() is called on it");
    doesnt_throw([](){ load(File("../test/seven.hacc")); }, "We can call load()");
    ok(File("../test/seven.hacc").loaded(), "File is loaded when load() is called on it");
    if (is(File("../test/seven.hacc").data().type, Type(typeid(int32)), "Loaded file preserves type")) {
        is(*(int32*)File("../test/seven.hacc").data().address, 7, "Loaded file preserves value");
    }
    else {
        diag(File("../test/seven.hacc").data().type.name().c_str());
        fail("Loaded file preserves value - failed because the type part failed");
    }
});
