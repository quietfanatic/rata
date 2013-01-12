#ifndef HAVE_HACC_HACCABLE_H
#define HAVE_HACC_HACCABLE_H

#include <unordered_map>
#include "hacc.h"
#include "getset.h"


namespace hacc {


 // This stores all the info for one type.
struct HaccTable {
    const std::type_info& cpptype;
    bool initialized = false;

     // How to allocate this type.  Generally set to "new X ()"
    Func<void* ()> allocate;
     // Pretty much no reason to override this.  It calls delete by default.
    Func<void (void*)> deallocate;
     // ID manipulation turns out to be important.
    Func<String (void*)> get_id_p;
    Func<void* (String)> find_by_id_p;
     // Leave haccification to something else.
     // If the inner type of this is const Hacc*, it defines a direct transition.
    GetSet0 delegate;
     // Defined by attributes with names, defaults are TODO
    Map<GetSet0> attrs;
     // Defined by a fixed number of elements
    VArray<GetSet0> elems;
     // Variants with names specific to this interface
     // Note that this will only be used if following a pointer.
    std::unordered_map<String, GetSet0> variants;
    Func<String (void*)> select_variant;
     // Manual direct translation
    Func<const Hacc* (void*)> to;
    Func<void (void*, const Hacc*)> update_from;
     // Records binary-compatible subtypes of the type pointed to by this type, under names.
    std::unordered_map<String, Caster0> subtypes;
     // Allows this to be treated as a pointer.
    GetSet0 pointer;
    bool follow_pointer;
     // These will be automatically set with pointer
    const std::type_info* pointee_type = null;
    const std::type_info* (* pointee_realtype ) (void*) = null;
    
    const Hacc* to_hacc (void*);
    const Hacc* to_hacc_inner (void*);
    void update_from_hacc (void*, const Hacc*);
    void* new_from_hacc (const Hacc* h);
    String get_id (void*);
    void* find_by_id (String);
    void* require_id (String);

     // This throws an error when not found
    static HaccTable* require_cpptype (const std::type_info&);
     // This autovivifies.
    static HaccTable* by_cpptype (const std::type_info&);
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
};
namespace hacc {

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
template <class C, bool has_nc = std::is_constructible<C>::value> struct per_nc;
template <class C> struct per_nc<C, true> { static void* allocate () { return new C; } };
template <class C> struct per_nc<C, false> { static void* allocate () { 
    throw Error("Cannot allocate <mangled: " + String(typeid(C).name()) + "> because it has no nullary constructor.");
} };

 // This class provides the DSL for creating HaccTables.
 //  Every specialization of Haccable must inherit from this.
template <class C> struct Haccability : GetSet_Builders<C> {
    
     // If a table was already allocated (possibly in another compilation unit),
     //  this returns that.  Otherwise, it builds one from C's HCB.
     // This needs to be, and I hope is, reentrant.
    static HaccTable* get_table () {
        static HaccTable* table = HaccTable::by_cpptype(typeid(C));
        if (!table->initialized) {
            table->initialized = true;
            table->allocate = per_nc<C>::allocate;
            table->deallocate = [](void* p){ delete (C*)p; };
            Haccable<C>::describe();
        }
        return table;
    }

     // Alright, here's the DSL for when you're defining haccabilities.
    static void allocate (const Func<C* ()>& f) { get_table()->allocate = *(Func<void* ()>*)&f; }
    static void deallocate (const Func<void (C*)>& f) { get_table()->deallocate = *(Func<void* ()>*)&f; }
    static void get_id (const Func<String (const C&)>& f) { get_table()->get_id_p = *(Func<String (void*)>*)&f; }
    static void find_by_id (const Func<C* (String)>& f) { get_table()->find_by_id_p = *(Func<void* (String)>*)&f; }
    static void to (const Func<const Hacc* (const C&)>& f) { get_table()->to = *(Func<const Hacc* (void*)>*)&f; }
    static void update_from (const Func<void (C&, const Hacc*)>& f) { get_table()->update_from = *(Func<void (void*, const Hacc*)>*)&f; }
    static void delegate (const GetSet1<C>& gs) { get_table()->delegate = gs; }
    static void attr (String name, const GetSet1<C>& gs) { get_table()->attrs.emplace_back(name, gs); }
    static void elem (const GetSet1<C>& gs) { get_table()->elems.push_back(gs); }
    static void variant (String name, const GetSet1<C>& gs) { get_table()->variants.emplace(name, gs); }
    static void select_variant (const Func<String (const C&)>& f) { get_table()->select_variant = *(Func<String (void*)>*)&f; }
    template <class B>
    static void base (String name) {
        Haccable<B>::get_table()->subtypes.emplace(name, Caster2<B, C>());
    }
    template <class P>
    static void follow_pointer (const GetSet2<C, P*>& gs) {
        get_table()->pointer = gs;
        get_table()->follow_pointer = true;
        get_table()->pointee_type = &typeid(P);
        get_table()->pointee_realtype = [](void* p) { return &typeid(*(P*)p); };
    }
    template <class P>
    static void reference_pointer (const GetSet2<C, P*>& gs) {
        get_table()->pointer = gs;
        get_table()->pointee_type = &typeid(P);
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
const Hacc* to_hacc (const C& v) {
    return require_hacctable<C>()->to_hacc((void*)&v);
}
template <class C>
const Hacc* hacc_from (const C& v) {
    return to_hacc(v);
}
template <class C>
void update_from_hacc (C& v, const Hacc* h) {
    return require_hacctable<C>()->update_from_hacc((void*)&v, h);
}
template <class C> C value_from_hacc (const Hacc* h) {
    C r;
    update_from_hacc(r, h);
    return r;
}
template <class C> C hacc_to_value (const Hacc* h) {
    C r;
    update_from_hacc(r, h);
    return r;
}
template <class C> C* new_from_hacc (const Hacc* h) {
    return (C*)require_hacctable<C>()->new_from_hacc(h);
}
template <class C> C* hacc_to_new (const Hacc* h) {
    return new_from_hacc<C>(h);
}

template <class C> String get_id (const C& v) {
    return require_hacctable<C>()->get_id((void*)&v);
}
template <class C> C* find_by_id (String id) {
    return (C*)require_hacctable<C>()->find_by_id(id);
}
template <class C> C* require_id (String id) {
    return (C*)require_hacctable<C>()->require_id(id);
}

}

 // I guess we need to use some macros after all.
 // These are for convenience in defining haccabilities.  The non-template version also forces
 //  instantiation of the haccability just defined.
#define HCB_COMMA ,  // because literal commas confuse function-like macros.
#define HCB_UNQ2(a, b) a##b
#define HCB_UNQ1(a, b) HCB_UNQ2(a, b)
#define HCB_INSTANCE(type) static bool HCB_UNQ1(_HACCABLE_instantiation_, __COUNTER__) = Haccable<type>::get_table();
#define HCB_BEGIN(type) template <> struct Haccable<type> : hacc::Haccability<type> { static void describe () {
#define HCB_END(type) } }; static bool HCB_UNQ1(_HACCABLE_instantiation_, __COUNTER__) = Haccable<type>::get_table();
#define HCB_PARAMS(...) __VA_ARGS__
#define HCB_TEMPLATE_BEGIN(params, type) template params struct Haccable<type> : hacc::Haccability<type> { \
    using hcb = hacc::Haccability<type>; \
    using hcb::allocate; \
    using hcb::get_id; \
    using hcb::find_by_id; \
    using hcb::to; \
    using hcb::update_from; \
    using hcb::delegate; \
    using hcb::attr; \
    using hcb::elem; \
    using hcb::variant; \
    using hcb::select_variant; \
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
    using hcb::follow_pointer; \
    using hcb::reference_pointer; \
    static void describe () {
#define HCB_TEMPLATE_END(params, type) } };  // Reserved in case we need to do some magic static-var wrangling


#endif
