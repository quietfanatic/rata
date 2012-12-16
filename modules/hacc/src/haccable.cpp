#include <unordered_map>
#include "../inc/haccable.h"


#define INIT_SAFE(name, ...) static __VA_ARGS__& name () { static __VA_ARGS__ r; return r; }

namespace hacc {

    // Hahaha, deleting 150 lines of code is so satisfying.

    INIT_SAFE(cpptype_map, std::unordered_map<const std::type_info*, HaccTable*>)

    HaccTable* HaccTable::by_cpptype (const std::type_info& t) {
        auto iter = cpptype_map().find(&t);
        return iter != cpptype_map().end()
            ? iter->second
            : null;
    }

    HaccTable::HaccTable (const std::type_info& t) : cpptype(t) {
        cpptype_map().emplace(&t, this);
    }

}
