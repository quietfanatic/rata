#include <unordered_map>
#include <stdexcept>
#include "../inc/haccable_files.h"

namespace hacc {

    std::unordered_map<String, Generic> files;
    Generic generic_from_file (String name) {
        Generic& r = files[name];
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
    void deallocate_file_object (String name) {
        Generic& g = files.at(name);
        HaccTable* table = HaccTable::require_cpptype(*g.cpptype);
        table->deallocate(g.p);
        files.erase(name);
    }

}

