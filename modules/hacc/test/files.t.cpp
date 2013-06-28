#include "../inc/files.h"

 // TODO: right now we're being naughty and relying on
 //  haccable.t.cpp having been linked in with this file

using namespace hacc;

#include "../../tap/inc/tap.h"
tap::Tester files_tester ("hacc/files", [](){
    using namespace tap;
    plan(3);
    doesnt_throw([](){ load(File("modules/hacc/test/seven.hacc")); }, "We can load files");
    if (is(File("modules/hacc/test/seven.hacc").data().type, Type(typeid(int32)), "Loaded file preserves type")) {
        is(*(int32*)File("modules/hacc/test/seven.hacc").data().address, 7, "Loaded file preserves value");
    }
    else {
        fail("Loaded file preserves value - failed because the type part failed");
    }
});
