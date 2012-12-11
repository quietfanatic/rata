#include "../inc/haccable_standard.h"

 // This defines haccabilities for most builtin types.

HACCABLE(hacc::Null, { d::like_null(); })  // nullptr by default
HACCABLE(hacc::Bool, { d::like_bool(); })  // bool by default
HACCABLE(char, { d::like_integer(); })
HACCABLE(int8, { d::like_integer(); })
HACCABLE(uint8, { d::like_integer(); })
HACCABLE(int16, { d::like_integer(); })
HACCABLE(uint16, { d::like_integer(); })
HACCABLE(int32, { d::like_integer(); })
HACCABLE(uint32, { d::like_integer(); })
HACCABLE(int64, { d::like_integer(); })
HACCABLE(uint64, { d::like_integer(); })
HACCABLE(hacc::Float, { d::like_float(); })  // float by default
HACCABLE(hacc::Double, { d::like_double(); })  // double by default
HACCABLE(hacc::String, { d::like_string(); })  // std::String by default




