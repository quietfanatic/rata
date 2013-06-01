#include <unordered_map>
#include <stdexcept>
#include "../inc/haccable_files.h"

namespace hacc {

    std::unordered_map<String, Hacc*> files;

    Hacc* read_file (String name) {
        Hacc*& r = files[name];
        if (r) return r;
        r = collapse_hacc(hacc_from_file(name));
        return r;
    }

}

