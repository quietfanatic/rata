#include "../inc/haccable_standard.h"

 // This defines haccabilities for most builtin types.
#define HCB_LIT(cpptype, form) \
HCB_BEGIN(cpptype) \
    to([](const cpptype& x){ return new_hacc(x); }); \
    update_from([](cpptype& x, const Hacc* h){ x = h->get_##form(); }); \
HCB_END(cpptype)

using namespace hacc;


HCB_LIT(hacc::Null, null)
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
HCB_LIT(hacc::String, string)

