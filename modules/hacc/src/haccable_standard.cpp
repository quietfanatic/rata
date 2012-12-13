#include "../inc/haccable_standard.h"

 // This defines haccabilities for most builtin types.
HACCABLE(hacc::Null, { d.hacctype("Null"); d.like_null(); })  // nullptr by default
HACCABLE(bool, { d.hacctype("bool"); d.like_bool(); })  // bool by default
HACCABLE(char, { d.hacctype("char"); d.like_integer(); })
HACCABLE(int8, { d.hacctype("int8"); d.like_integer(); })
HACCABLE(uint8, { d.hacctype("uint8"); d.like_integer(); })
HACCABLE(int16, { d.hacctype("int16"); d.like_integer(); })
HACCABLE(uint16, { d.hacctype("uint16"); d.like_integer(); })
HACCABLE(int32, { d.hacctype("int32"); d.like_integer(); })
HACCABLE(uint32, { d.hacctype("uint32"); d.like_integer(); })
HACCABLE(int64, { d.hacctype("int64"); d.like_integer(); })
HACCABLE(uint64, { d.hacctype("uint64"); d.like_integer(); })
HACCABLE(float, { d.hacctype("float"); d.like_float(); })
HACCABLE(double, { d.hacctype("double"); d.like_double(); })
HACCABLE(hacc::String, { d.hacctype("String"); d.like_string(); })  // std.String by default




