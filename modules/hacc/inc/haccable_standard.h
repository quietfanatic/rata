
#include "haccable.h"
#include "string.h"

HACCABLE_TEMPLATE_BEGIN(<class C>, C*) {
    get_type([](C* const& v) {
        if (v)
            return hacc::get_type(*v) + "*";
        else
            return std::string("<get_type on null pointer NYI>");
    });
    to([](C* const& v){
        if (v) {
            char id [17];
            sprintf(id, "%lx", (unsigned long)v);
            std::string type = hacc::has_type(*v) ? hacc::get_type(*v) : "";
            return hacc::Hacc(hacc::Ref{type, std::string(id, strlen(id))});
        }
        else {
            return hacc::Hacc(hacc::null);
        }
    });
    update_from([](C*& p, hacc::Hacc h){
        if (h.valtype() == hacc::VALNULL) {
            p = nullptr;
        }
        else {
             // NOTE: We aren't fixing ids right now,
             //  so this address had better be valid!
            sscanf(h.get_ref().id.c_str(), "%lx", (unsigned long*)&p);
        }
    });
} HACCABLE_TEMPLATE_END(<class C>, C*)

