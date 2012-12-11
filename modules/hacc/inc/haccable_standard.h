
#include "haccable.h"
#include "string.h"

HACCABLE_TEMPLATE_BEGIN(<class C>, C*) {
    using namespace hacc;
    d::get_type([](C* const& v) {
        if (v)
            return get_type(*v) + "*";
        else
            return String("<get_type on null pointer NYI>");
    });
    d::to([](C* const& v){
        if (v) {
            char id [17];
            sprintf(id, "%lx", (unsigned long)v);
            String type = has_type(*v) ? get_type(*v) : "";
            return Hacc(Ref{type, String(id, strlen(id))});
        }
        else {
            return Hacc(null);
        }
    });
    d::update_from([](C*& p, Hacc h){
        if (h.valtype() == VALNULL) {
            p = nullptr;
        }
        else {
             // NOTE: We aren't fixing ids right now,
             //  so this address had better be valid!
            sscanf(h.get_ref().id.c_str(), "%lx", (unsigned long*)&p);
        }
    });
} HACCABLE_TEMPLATE_END(<class C>, C*)

