#ifndef HAVE_HACC_HACCABLE_POINTERS
#define HAVE_HACC_HACCABLE_POINTERS

#include <memory>
#include "haccable.h"


namespace hacc {
     // This is a pointer that will be followed when haccing; the data it points to
     //  will be included in the Hacc, and future pointers that are identical will
     //  reference it.  If the data pointer to is polymorphic, follow_ptr will be
     //  hacced as though it is a union of all of C's subtypes.
     //
     // Using update_from_hacc on a follow_ptr that already has data pointing to
     //  it will not delete the pointed-to data.  If the data is not deleted manually
     //  or through canonical_ptr, it will leak.
    template <class C>
    struct follow_ptr {
        C* p;
        follow_ptr (C* p = null) : p(p) { }
        operator C* () const { return p; }
        C& operator * () const { return *p; }
        C* operator -> () const { return p; }
    };
     // A canonical_ptr is the same as a follow_ptr, both in C++ and in HACC, except
     //  that it will delete its data when it is destroyed or when update_from_hacc
     //  is called on it.
    template <class C>
    struct canonical_ptr {
        C* p;
        canonical_ptr (C* p = null) : p(p) { }
        operator C* () const { return p; }
        C& operator * () const { return *p; }
        C* operator -> () const { return p; }
        ~canonical_ptr () { if (p) { delete p; p = null; } }
        canonical_ptr& operator = (C* newp) { if (p) delete p; p = newp; return *this; }
        canonical_ptr (canonical_ptr&& o) { o.~canonical_ptr(); operator=(o.p); }
    };

}

HCB_TEMPLATE_BEGIN(<class C>, hacc::follow_ptr<C>)
     // BLERGH!  What the heck, C++, making me stick 'template' in the middle of this
    type_name("hacc::follow_ptr<" + hacc::get_type_name<C>() + ">");
    pointer(hacc::Haccability<hacc::follow_ptr<C>>::template assignable<C*>());
    pointer_policy(hacc::FOLLOW);
HCB_TEMPLATE_END(<class C>, hacc::follow_ptr<C>)

HCB_TEMPLATE_BEGIN(<class C>, hacc::canonical_ptr<C>)
    type_name("hacc::canonical_ptr<" + hacc::get_type_name<C>() + ">");
    pointer(hacc::Haccability<hacc::canonical_ptr<C>>::template assignable<C*>());
    pointer_policy(hacc::FOLLOW);
HCB_TEMPLATE_END(<class C>, hacc::canonical_ptr<C>)

// TODO: Add shared_ptr and unique_ptr, similar to the above

 // Vanilla pointers will never be followed.  They will be represented in a Hacc as
 //  an ID, which will be generated either from the pointed class's HCB's get_id(), or
 //  by the address of the object.  The former case is useful if the ID represents
 //  a static resource that you don't want to serialize with your data.  The latter case
 //  will happen if the pointed-to data is serialized elsewhere.
 // NOTE: If the pointed-to data is not serialized elsewhere or locatable through find_by_id,
 //  then the resulting HACC will not be able to be read back in.

HCB_TEMPLATE_BEGIN(<class C>, C*)
    type_name(hacc::get_type_name<C>() + "*");
    pointer(hacc::Haccability<C*>::template supertype<C*>());
    pointer_policy(hacc::ASK_POINTEE);
HCB_TEMPLATE_END(<class C>, C*)

#endif
