
#include "../inc/everything.h"


#include "../../tap/inc/tap.h"
tap::Tester everything_tester ("hacc-everything", [](){
    using namespace hacc;
    using namespace tap;
    plan(2);
    is(to_string(std::vector<int>{3, 4, 5, 6, 7, 8}), String("[3, 4, 5, 6, 7, 8]"), "std::vector to string");
    is(to_string(std::unordered_map<String, float>{std::pair<String, float>("asfd", -1234), std::pair<String, float>("qabl", 900)}), String("{qabl: 900~44610000, asfd: -1234~c49a4000}"), "std::unordered_map to string");
});
