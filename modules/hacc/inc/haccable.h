#ifndef HAVE_HACC_HACCABLE_H
#define HAVE_HACC_HACCABLE_H

#include <unordered_map>
#include "hacc.h"
#include "haccable_getset.h"

namespace hacc {

enum Pointer_Policy {
    ASK_POINTEE,
    REFERENCE,
    FOLLOW,
    ALWAYS_FOLLOW
};


 // This stores all the info for one type.
struct HaccTable {
    const std::type_info& cpptype;
    size_t cppsize;
    bool initialized = false;

     // Mostly makes error reporting much nicer.
    String type_name;
     // These are automatically populated
    Func<void (void*)> construct;
    Func<void (void*, void*)> assign;
    Func<void (void*)> destroy;
     // Leave haccification to something else.
     // If the inner type of this is Hacc*, it defines a direct transition.
    GetSet0 delegate;
     // Defined by attributes with names, defaults are TODO
    Map<GetSet0> attrs;
    Func<Pointer (void*, String)> get_attr_p;
     // Defined by a fixed number of elements
    VArray<GetSet0> elems;
    Func<Pointer (void*, size_t)> get_elem_p;
     // Variants with names specific to this interface
     // Note that this will only be used if following a pointer.
    std::unordered_map<String, GetSet0> variants;
    Func<String (void*)> select_variant;
     // Manual direct translation
    Func<Hacc* (void*)> to;
    Func<void (void*, Hacc*)> update_from;
     // Records binary-compatible subtypes of the type pointed to by this type, under names.
    std::unordered_map<String, Caster0> subtypes;
     // Allows this to be treated as a pointer.
    GetSet0 pointer;
    uint8 pointer_policy = ASK_POINTEE;
    uint8 pointee_policy = REFERENCE;  // This can be set on the pointee's hacctable.
     // These will be automatically set with pointer
    const std::type_info* pointee_type = null;
    const std::type_info* (* pointee_realtype ) (void*) = null;
     // This allows the type to accept and empty array or hash
    bool empty = false;
     // These allow the type to be treated kinda like an enum
    std::unordered_map<String, Defaulter0> values;
    Func<String (void*)> value_name;
     // Call this after every *_from_hacc
    Func<void (void*)> finish;
    
    Hacc* to_hacc (void*);
    void update_from_hacc (void*, Hacc*, bool save_name = true);
    void* new_from_hacc (Hacc* h);
     // Helpers that I'd like to not have to declare here but C++.
    Hacc* to_hacc_inner (void*);
    void update_from_hacc_inner (void*, Hacc*);
    void update_with_getset (void*, Hacc*, const GetSet0&);
    uint8 get_pointer_policy ();
    String get_type_name ();

     // Goes through attrs or get_attr_p
    Pointer get_attr (void*, String);
     // Goes through elems or get_elem_p
    Pointer get_elem (void*, size_t);

     // This throws an error when not found
    static HaccTable* require_cpptype (const std::type_info&);
     // This autovivifies.
    static HaccTable* by_cpptype (const std::type_info&);

    static HaccTable* require_type_name (String);
    void reg_type_name (String);
    HaccTable (const std::type_info& t);
};

}  // Haccable is outside the namespace for specialization convenience

 // This is what you specialize in order to make a type haccable.
 // Getting tables for types whose types are known at compile-time goes through here.
template <class C> struct Haccable {
     // The default get_table looks for tables defined in other compilation units.
    static hacc::HaccTable* get_table () {
        static hacc::HaccTable* table = hacc::HaccTable::by_cpptype(typeid(C));
        return table;
    }
    static constexpr bool defined_here = false;
};

namespace hacc {

    template <class C> inline void instantiate_table () { Haccable<C>::get_table(); }

 // This class is essential enough to be included in every Haccable.
 //  It represents the canonical path to a polymorphic object.
 //  Basically, it's an auto_ptr, without the auto part. :)
 //  It has builtin haccability of a variant type, which is automatically
 //  built up from 'base' declarations in its variants' HCBs.
template <class C> struct PolyP {
    C* p;
    operator C*& () { return p; }
    C& operator * () { return *p; }
    C& operator -> () { return *p; }
    PolyP (C* p = NULL) : p(p) { }

    static std::unordered_map<std::string, GetSet0>*& variants () {
        static std::unordered_map<std::string, GetSet0> variants_pre;
        static std::unordered_map<std::string, GetSet0>* variants = &variants_pre;
        return variants;
    }
};


 // Select based on whether C has a nullary constructor.
template <class C, bool has_nc = std::is_constructible<C>::value> struct constructibility;
template <class C> struct constructibility<C, true> { static void construct (void* p) { new (p) C; } };
template <class C> struct constructibility<C, false> { static void construct (void* p) { 
    throw Error("Cannot construct " + Haccable<C>::get_table()->get_type_name() + " because it has no nullary constructor.");
} };
template <class C, bool a = std::is_assignable<C&, const C&>::value> struct assignability;
template <class C> struct assignability<C, true> { static void assign (void* l, void* r) { *(C*)l = *(C*)r; } };
template <class C> struct assignability<C, false> { static void assign (void* l, void* r) {
    throw Error("Cannot assign " + Haccable<C>::get_table()->get_type_name() + " because it is not assignable to itself.");
} };

 // This class provides the DSL for creating HaccTables.
 //  Every specialization of Haccable must inherit from this.
template <class C> struct Haccability : GetSet_Builders<C> {
    static constexpr bool defined_here = true;
    
     // If a table was already allocated (possibly in another compilation unit),
     //  this returns that.  Otherwise, it builds one from C's HCB.
     // This needs to be, and I hope is, reentrant.
    static HaccTable* get_table () {
        static HaccTable* table = HaccTable::by_cpptype(typeid(C));
        if (!table->initialized) {
            table->initialized = true;
            table->cppsize = sizeof(C);
            table->construct = constructibility<C>::construct;
            table->assign = assignability<C>::assign;
            table->destroy = [](void* p){ ((C*)p)->~C(); };
            Haccable<C>::describe();
        }
        return table;
    }

     // Alright, here's the DSL for when you're defining haccabilities.
    static void type_name (String name) {
        if (get_table()->type_name.empty())
            get_table()->type_name = name;
        get_table()->reg_type_name(name);
     }
    static void finish (const Func<void (C&)>& f) { get_table()->finish = *(Func<void (void*)>*)&f; }
    static void to (const Func<Hacc* (const C&)>& f) { get_table()->to = *(Func<Hacc* (void*)>*)&f; }
    static void update_from (const Func<void (C&, Hacc*)>& f) { get_table()->update_from = *(Func<void (void*, Hacc*)>*)&f; }
    static void delegate (const GetSet1<C>& gs) { get_table()->delegate = gs; }
    static void attr (String name, const GetSet1<C>& gs) { get_table()->attrs.emplace_back(name, gs); }
    static void get_attr (const Func<Pointer (C&, String)>& f) { get_table()->get_attr_p = *(Func<Pointer (void*, String)>*)&f; }
    static void elem (const GetSet1<C>& gs) { get_table()->elems.push_back(gs); }
    static void get_elem (const Func<Pointer (C&, size_t)>& f) { get_table()->get_elem_p = *(Func<Pointer (void*, size_t)>*)&f; }
    static void variant (String name, const GetSet1<C>& gs) { get_table()->variants.emplace(name, gs); }
    static void select_variant (const Func<String (const C&)>& f) { get_table()->select_variant = *(Func<String (void*)>*)&f; }
    static void value (String name, const C& val) { get_table()->values.emplace(name, GetSet_Builders<C>::def(val)); }
    static void value_name (const Func<String (const C&)>& f) { get_table()->value_name = *(Func<String (void*)>*)&f; }
    static void empty () { get_table()->empty = true; }
    template <class B>
    static void base (String name) {
        Haccable<B>::get_table()->subtypes.emplace(name, Caster2<B, C>());
    }
    template <class P>
    static void pointer (const GetSet2<C, P*>& gs) {
        get_table()->pointer = gs;
        get_table()->pointee_type = &typeid(P);
        get_table()->pointee_realtype = [](void* p) { return &typeid(*(P*)p); };
    }
    static void pointer_policy (uint8 policy) {
        get_table()->pointer_policy = policy;
    }
    static void pointee_policy (uint8 policy) {
        get_table()->pointee_policy = policy;
    }
};

template <class C>
HaccTable* require_hacctable () {
    HaccTable* t = Haccable<C>::get_table();
    if (!t) throw Error("Unhaccable type <mangled: " + String(typeid(C).name()) + ">.");
    return t;
}

 // Finally, the API you use to convert things to and from Haccs.
 // Do not call these at init-time.  Haccability descriptions in other compilation units
 //  will not have been run yet.

template <class C>
Hacc* to_hacc (const C& v) {
    return require_hacctable<C>()->to_hacc((void*)&v);
}
template <class C>
Hacc* hacc_from (const C& v) {
    return to_hacc(v);
}
template <class C>
void update_from_hacc (C& v, Hacc* h) {
    return require_hacctable<C>()->update_from_hacc((void*)&v, h);
}
template <class C> C value_from_hacc (Hacc* h) {
    C r;
    require_hacctable<C>()->update_from_hacc((void*)&r, h, false);
    return r;
}
template <class C> C hacc_to_value (Hacc* h) {
    C r;
    require_hacctable<C>()->update_from_hacc((void*)&r, h, false);
    return r;
}
template <class C> C* new_from_hacc (Hacc* h) {
    return (C*)require_hacctable<C>()->new_from_hacc(h);
}
template <class C> C* hacc_to_new (Hacc* h) {
    return new_from_hacc<C>(h);
}

template <class C> String get_type_name () {
    return Haccable<C>::get_table()->get_type_name();
}

}

 // I guess we need to use some macros after all.
 // These are for convenience in defining haccabilities.  The non-template version also forces
 //  instantiation of the haccability just defined.
#define HCB_COMMA ,  // because literal commas confuse function-like macros.
#define HCB_UNQ2(a, b) a##b
#define HCB_UNQ1(a, b) HCB_UNQ2(a, b)
#define HCB_INSTANCE(type) static bool HCB_UNQ1(_HACCABLE_instantiation_, __COUNTER__) __attribute__ ((unused)) = Haccable<type>::get_table();
#define HCB_BEGIN(type) template <> struct Haccable<type> : hacc::Haccability<type> { static void describe () {
#define HCB_END(type) } }; static bool HCB_UNQ1(_HACCABLE_instantiation_, __COUNTER__) __attribute__ ((unused)) = Haccable<type>::get_table();
#define HCB_PARAMS(...) __VA_ARGS__
#define HCB_TEMPLATE_BEGIN(params, type) template params struct Haccable<type> : hacc::Haccability<type> { \
    using hcb = hacc::Haccability<type>; \
    using hcb::type_name; \
    using hcb::finish; \
    using hcb::to; \
    using hcb::update_from; \
    using hcb::delegate; \
    using hcb::attr; \
    using hcb::get_attr; \
    using hcb::elem; \
    using hcb::get_elem; \
    using hcb::variant; \
    using hcb::select_variant; \
    using hcb::value; \
    using hcb::value_name; \
    using hcb::empty; \
    using hcb::base; \
    using hcb::value_functions; \
    using hcb::ref_functions; \
    using hcb::ref_function; \
    using hcb::assignable; \
    using hcb::supertype; \
    using hcb::member; \
    using hcb::value_methods; \
    using hcb::ref_methods; \
    using hcb::ref_method; \
    using hcb::pointer; \
    using hcb::pointer_policy; \
    using hcb::pointee_policy; \
    static void describe () {
#define HCB_TEMPLATE_END(params, type) } };  // Reserved in case we need to do some magic static-var wrangling

namespace hacc {
     // This does various transformations internal to the HACC language.
    Hacc* collapse_hacc (Hacc* h, Hacc** return_incantion = NULL);

     // In order for saved information to reference static information in files,
     //  we provide a system to register arbitrary pointers with the Hacc that
     //  generated them.  This is for things like file("tex.image").ALL
    void record_incantation (void*, Hacc*);
    Hacc* remember_incantation (void*);
    void clear_incantations ();
    static inline void deallocate_bundled_files () { clear_incantations(); }
}

#endif
