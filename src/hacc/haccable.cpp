
#ifdef HEADER

 // So I wanted to do this whole thing the "proper" way, without relying on RTTI.
 // That way, for you to implement per-class haccability, I was gonna have you
 // specialize a Haccable<C> pseudo-namespace with static functions that'd override
 // its inherited Haccability<C>'s function pointers, thus populating a per-class
 // Generic_Haccability vtable that'd be referenced by a Haccable_Ptr fat pointer,
 // that would stack up static_casts in lambdas to allow derived-to-base pointer
 // conversion to hacc a derived type from a pointer to a base type.
        #include <typeinfo>
 // Aren't you glad I got over my hatred of RTTI instead?


 // Well, you're still gonna populate a pseudo-namespace with static functions,
 // but that shouldn't be hard to figure out.  Basically, for any type you're gonna:
 //
 // template <>
 // struct Haccable<My_Thing> : Haccabililty<My_Thing> {
 //     String hacctype = "My_Thing";
 //     static Hacc to (const My_Thing& thing) {
 //         return Hacc(Object({
 //             Pair("x", to_hacc(thing.get_x())),
 //             Pair("y", to_hacc(thing.get_y())),
 //         });
 //     }
 //     static My_Thing from (Hacc hacc) {
 //         return My_Thing(
 //             from_hacc<int>(hacc.find("x")),
 //             from_hacc<int>(hacc.find("y"))
 //         );
 //     }
 // }
 //
 // Well, that's a bit more clunky than I'd hoped, but once we get the object
 // syntax sugar happening it'll be nicer.

#include <stdexcept>

 // Specialize this and make it inherit from Haccabililty<C>
 // For convenience it is not in the hacc namespace because the compiler doesn't
 // like you specializing things from a different namespace.
template <class C>
struct Haccable;

namespace hacc {

 // We're assuming references and pointers are binary-compatible.
using to_f = Hacc (const void*);
using update_f = void (void*, Hacc);
using new_f = void* (Hacc);


struct Generic_Haccability {
    String hacctype;
    String cpptype;  // Non-portable, so don't look! :)
    to_f* to;
     // can't genericize from_hacc, unfortunately
    update_f* update_from;
    new_f* new_from;

    static Hash<Generic_Haccability*> by_hacctype;
    static Hash<Generic_Haccability*> by_cpptype;
    Generic_Haccability (
        String hacctype,
        String cpptype,
        to_f* to,
        update_f* update_from,
        new_f* new_from
    ) :
        hacctype(hacctype), cpptype(cpptype), to(to), update_from(update_from), new_from(new_from)
    {
        if (!hacctype.empty()) {
            char* cstr = new char [hacctype.length() + 1];
            memcpy(cstr, hacctype.data(), hacctype.length());
            cstr[hacctype.length()] = 0;
            by_hacctype.insert(cstr, this);
        }
        char* cstr = new char [cpptype.length() + 1];
        memcpy(cstr, cpptype.data(), cpptype.length());
        cstr[cpptype.length()] = 0;
        by_cpptype.insert(cstr, this);
    }
};

 // Now the usage API, which is comparatively so much simpler.

template <class C>
Hacc to_hacc (const C& thing) { return Haccable<C>::to(thing); }
template <class C>
C from_hacc (Hacc hacc) { return Haccable<C>::from(hacc); }


template <class C>
struct Haccability {
     // The class doesn't actually have to have a type name.
    static String hacctype;
     // Provide defaults
    static Hacc to (const C& thing) {
        throw std::logic_error(
              "No to_hacc was defined for hacctype "
            + hacctype
            + " (C++ mangled type name:"
            + typeid(C).name()
            + ")"
        );
    }
    static C from (Hacc hacc) {
        throw std::logic_error(
              "No from_hacc was defined for hacctype "
            + hacctype
            + " (C++ mangled type name:"
            + typeid(C).name()
            + ")"
        );
    }
    static void update_from (C& thing, Hacc hacc) {
        thing = from_hacc<C>(hacc);
    }
    static C* new_from (Hacc hacc) {
        return new C (from_hacc<C>(hacc));
    }

     // Erase the type information
    static Generic_Haccability generic;
};

 // Create
template <class C> Generic_Haccability Haccability<C>::generic (
    Haccable<C>::hacctype, typeid(C),
    reinterpret_cast<to_f*>(Haccable<C>::to),
    reinterpret_cast<update_f*>(Haccable<C>::update_from),
    reinterpret_cast<new_f*>(Haccable<C>::new_from)
); 

}

#else

#ifndef DISABLE_TESTS

Tester haccable_tester ("haccable", [](){
    plan(1);
    pass("Start testing haccables.");
});

#endif

#endif
