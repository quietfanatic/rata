
#include <unordered_map>
#include "../inc/regions.h"
#include "../inc/hacc.h"

namespace hacc {

    struct Region {
        std::unordered_map<String, uint> file_cache;
        std::vector<Generic> allocations;
    };
    Region _permanent;
    Region* permanent = &_permanent;

    Generic generic_from_file (String name, Region* region) {
        auto iter = region->file_cache.find(name);
        if (iter != region->file_cache.end())
            return allocations[iter->second];
        else Generic r;
        Generic& r = region->file_cache[name];
        if (r.p) return r;
        Hacc* h = hacc_from_file(name);
        if (h->form() == ERROR) throw h->get_error();
        if (h->type.empty()) throw Error("Contents of file \"" + name + "\" did not give a type annotation.");
        HaccTable* table = HaccTable::require_type_name(h->type);
        void* p = table->new_from_hacc(h);
        r.cpptype = &table->cpptype;
        r.p = p;
        return r;
    }

}
