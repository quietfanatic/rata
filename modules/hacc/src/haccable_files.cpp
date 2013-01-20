#include <unordered_map>
#include <stdexcept>
#include "../inc/haccable_files.h"

namespace hacc {

    std::unordered_map<String, Generic> files;
    Generic generic_from_file (String name) {
        Generic& r = files[name];
        if (r.p) return r;
        Hacc* h = hacc_from_file(name);
        try {
            if (h->type.empty()) throw Error("Contents of file \"" + name + "\" did not give a type annotation.");
            HaccTable* table = HaccTable::require_type_name(h->type);
            void* p = table->new_from_hacc(h);
            r.cpptype = &table->cpptype;
            fprintf(stderr, "Loaded file of type %s\n", r.cpptype->name());
            r.p = p;
        } catch (std::exception& e) {
            h->destroy();
            delete h;
            fprintf(stderr, "Failed to load file: %s\n", e.what());
            throw e;
        }
        h->destroy();
        delete h;
        return r;
    }

}

