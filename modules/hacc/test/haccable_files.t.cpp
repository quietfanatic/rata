
#include "../inc/haccable_files.h"
#include "../inc/haccable_integration.h"
#include "../inc/haccable_standard.h"
#include "../inc/haccable_pointers.h"

HCB_INSTANCE(std::unordered_map<std::string HCB_COMMA int32>)

#include "../../tap/inc/tap.h"

tap::Tester tester ("haccable_files", [](){
    using namespace tap;
    using namespace hacc;
    plan(2);
    doesnt_throw([](){generic_from_file("/home/lewis/stash/git/rata/modules/hacc/test/test.hacc"); }, "Can use generic_from_file()\n");
    is(*value_from_string<int32*>("file(\"/home/lewis/stash/git/rata/modules/hacc/test/test.hacc\").asdf"), (int32)47, "file(\"name\").attr seems to work");
});
