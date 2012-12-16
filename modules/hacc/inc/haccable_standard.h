#ifndef HAVE_HACC_HACCABLE_STANDARD_H
#define HAVE_HACC_HACCABLE_STANDARD_H

#include "haccable.h"
/*
HACCABLE_TEMPLATE_BEGIN(<class C>, C*) {
    using namespace hacc;
    d.hacctype([](){
        return hacctype<C>() + "*";
    });
    d.to_hacc([](C* const& v){
        if (v) {
            String type = has_hacctype(*v) ? hacctype(*v) : "";
            return Hacc(Ref{type, address_to_id((void*)v)});
        }
        else {
            return Hacc(null);
        }
    });
    d.from_hacc([](Hacc h){
        if (h.valtype() == VALNULL) {
            return (C*)null;
        }
        else {
             // NOTE: We aren't fixing ids right now,
             //  so this address had better be valid!
            return (C*)id_to_address(h.get_ref().id);
        }
    });
} HACCABLE_TEMPLATE_END(<class C>, C*)
*/

#endif
