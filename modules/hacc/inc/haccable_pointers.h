#ifndef HAVE_HACCABLE_POINTERS
#define HAVE_HACCABLE_POINTERS

#include <memory>
#include "haccable.h"


 // This is a pointer that will be followed when haccing as if it were not there,
 //  with the exception that if the C is polymorphic, canonical_ptr will be hacced
 //  as though it is a union of all of C's subtypes.
 //
 // unique_ptr is treated exactly the same.  Unlike unique_ptr, canonical_ptr can
 //  be casted freely to and from an unannotated pointer.  It does, however, delete
 //  the pointed-to data when it is destroyed or a new pointer is assigned to it.
namespace hacc {
    template <class C>
    struct canonical_ptr {
        C* p = null;
        canonical_ptr (C* p) : p(p) { }
        ~canonical_ptr () { if (p) { delete p; p = null; } }
        canonical_ptr& operator = (C* newp) { if (p) delete p; p = newp; }
        canonical_ptr (canonical_ptr&& o) { o.~canonical_ptr(); operator=(o.p); }
        operator C*& () { return p; }
        C& operator * () { return *p; }
        C& operator -> () { return *p; }
    };
}

HCB_TEMPLATE_BEGIN(<class C>, hacc::canonical_ptr<C>)
    canonical_pointer(coerce<C*>());
HCB_TEMPLATE_END(<class C>, hacc::canonical_ptr<C>)

HCB_TEMPLATE_BEGIN(<class C>, std::unique_ptr<C>)
    canonical_pointer(value_functions(
        [](const std::unique_ptr<C>& up){ return up.get(); },
        [](std::unique_ptr<C>& up, C*const& p){ up.reset(); up = std::unique_ptr<C>(p); }
    ));
HCB_TEMPLATE_END(<class C>, std::unique_ptr<C>)

 // TODO: shared_ptr and a ptr class with similar but non-strict semantics

#endif
