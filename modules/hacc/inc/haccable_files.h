#ifndef HAVE_HACC_HACCABLE_FILES_H
#define HAVE_HACC_HACCABLE_FILES_H

#include "haccable_integration.h"

namespace hacc {

     // These functions cache the file's contents.

    Hacc* read_file (String name);

    template <class C> C file (String name) {
        return value_from_hacc<C>(read_file(name));
    }

    template <class C> C* reference_file (String name) {
        return file<C*>(name);
    }

}


#endif
