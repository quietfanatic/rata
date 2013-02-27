#ifndef HAVE_HACC_HACCABLE_FILES_H
#define HAVE_HACC_HACCABLE_FILES_H

#include "haccable_integration.h"

namespace hacc {

    Generic generic_from_file (String name);

    template <class C> C* reference_file (String name) {
        Generic g = generic_from_file(name);
        if (*g.cpptype == typeid(C)) {
            return (C*)g.p;
        }
        else throw Error("Requested file \"" + name + "\" is not of type " + get_type_name<C>() + ".");
    }
    void deallocate_file_object (String name);

}


#endif
