#include "../inc/haccable_standard.h"

 // This defines haccabilities for most builtin types.

HACCABLE(hacc::Null, { like_null(); })  // nullptr by default
HACCABLE(hacc::Bool, { like_bool(); })  // bool by default
HACCABLE(char, { like_integer(); })
HACCABLE(int8, { like_integer(); })
HACCABLE(uint8, { like_integer(); })
HACCABLE(int16, { like_integer(); })
HACCABLE(uint16, { like_integer(); })
HACCABLE(int32, { like_integer(); })
HACCABLE(uint32, { like_integer(); })
HACCABLE(int64, { like_integer(); })
HACCABLE(uint64, { like_integer(); })
HACCABLE(hacc::Float, { like_float(); })  // float by default
HACCABLE(hacc::Double, { like_double(); })  // double by default
HACCABLE(hacc::String, { like_string(); })  // std::String by default




