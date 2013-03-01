
#include "../inc/haccable_files.h"
#include "../inc/haccable_integration.h"
#include "../inc/haccable_standard.h"
#include "../inc/strings.h"

HCB_INSTANCE(std::unordered_map<std::string HCB_COMMA int32>)

#include "../../tap/inc/tap.h"

int32* p;

tap::Tester tester ("haccable_files", [](){
    using namespace tap;
    using namespace hacc;
    plan(4);
    doesnt_throw([](){read_file("/home/lewis/stash/git/rata/modules/hacc/test/test.hacc"); }, "Can use read_file()\n");
    doesnt_throw([](){p = value_from_string<int32*>("file(\"/home/lewis/stash/git/rata/modules/hacc/test/test.hacc\").asdf");}, "file(\"name\").attr doesn't break");
    is(*p, (int32)47, "...and it seems to work.");
    is(string_from(p), string("file(\"/home/lewis/stash/git/rata/modules/hacc/test/test.hacc\").asdf"), "We can remember the incantation required to get something from a file.");
});
