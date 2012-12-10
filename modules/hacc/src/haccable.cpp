
#include <
#include <unordered_map>

namespace hacc {
    std::unordered_map<const type_info&, HaccTable*> HaccTable::by_cpptype;
    std::unordered_map<const type_info&, HaccTable*> HaccTable::by_type;
}
