#ifndef HAVE_HACC_HACCABLE_STANDARD_H
#define HAVE_HACC_HACCABLE_STANDARD_H

#include "haccable.h"

template <class C> struct Haccable<C*> : hacc::Haccability<C*> {
    void describe (hacc::Haccer& h, C*& it) {
        h.as_pointer(it);
    }
};

#endif
