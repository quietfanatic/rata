#include "../inc/haccable_standard.h"

 // This defines haccabilities for most builtin types.
#define HACCABLE_LIT(cpptype, valtype) \
template <> struct Haccable<cpptype> : hacc::Haccability<cpptype> { \
    void describe (hacc::Haccer& h, cpptype& it) { h.as_##valtype(it); } \
};

HACCABLE_LIT(hacc::Null, null)
HACCABLE_LIT(bool, bool)
HACCABLE_LIT(char, integer)
HACCABLE_LIT(int8, integer)
HACCABLE_LIT(uint8, integer)
HACCABLE_LIT(int16, integer)
HACCABLE_LIT(uint16, integer)
HACCABLE_LIT(int32, integer)
HACCABLE_LIT(uint32, integer)
HACCABLE_LIT(int64, integer)
HACCABLE_LIT(uint64, integer)
HACCABLE_LIT(hacc::String, string)

