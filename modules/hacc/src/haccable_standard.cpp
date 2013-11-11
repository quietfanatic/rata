#include "../inc/haccable_standard.h"

 // This defines haccabilities for most builtin scalar types.
#define HCB_LIT(cpptype, form) \
HACCABLE(cpptype) { \
    name(#cpptype); \
    to_tree([](const cpptype& x){ return Tree(x); }); \
    fill([](cpptype& x, Tree t){ x = t.as<cpptype>(); }); \
}

using namespace hacc;

HACCABLE(std::nullptr_t) {
    name("std::nullptr_t");
    to_tree([](const std::nullptr_t& x){ return Tree(x); });
    fill([](std::nullptr_t& x, Tree t){ });
}

HCB_LIT(bool, bool)
HCB_LIT(char, integer)
HCB_LIT(int8, integer)
HCB_LIT(uint8, integer)
HCB_LIT(int16, integer)
HCB_LIT(uint16, integer)
HCB_LIT(int32, integer)
HCB_LIT(uint32, integer)
HCB_LIT(int64, integer)
HCB_LIT(uint64, integer)
HCB_LIT(float, float)
HCB_LIT(double, double)
HCB_LIT(std::string, string)

HCB_INSTANCE(std::unordered_map<std::string HCB_COMMA Dynamic>)
