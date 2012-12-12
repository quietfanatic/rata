
#include "../inc/hacc.h"



#include "../../tap/inc/tap.h"
tap::Tester hacc_options_tester ("hacc-options", [](){
    using namespace hacc;
    using namespace tap;
    plan(5);
    is(hf::default_options.unix_newlines(), true, "unix_newlines is on by default");
    is(hf::default_options.json_format(), false, "json_format is off by default");
    is((hf::default_options | hf::dos_newlines).unix_newlines(), false, "dos_newlines turns off unix_newlines");
    is((hf::indent_levels(4)|hf::indent_levels(7)).indent_levels(), 7, "indent_levels can be overridden");
    is((hf::indent_string("  ")|hf::indent_string("\t")).indent_string(), "\t", "indent_string can be overridden");
});









