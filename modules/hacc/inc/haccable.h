#ifndef HAVE_HACC_HACCABLE_H
#define HAVE_HACC_HACCABLE_H

#include <unordered_map>
#include "hacc.h"
#include "getset.h"


namespace hacc {


 // This stores all the info for one type.
struct HaccTable {
    const std::type_info& cpptype;

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
    Map<GetSet0> variants;
    Func<String (void*)> select_variant;
     // Direct translation
    Func<const Hacc* (void*)> to;
    Func<void (void*, const Hacc*)> update_from;
    
    const Hacc* to_hacc (void*);
    void update_from_hacc (void*, const Hacc*);
    void* new_from_hacc (const Hacc* h);
    String get_id (void*);
    void* find_by_id (String);
    void* require_id (String);

     // Shortcut for allocate and update_from_hacc.
    void* manifest ();

     // Creating and accessing hacctables.
    static HaccTable* by_cpptype (const std::type_info&);
    static HaccTable* require_cpptype (const std::type_info&);
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

 // Select based on whether C has a nullary constructor.
template <class C, bool has_nc = std::is_constructible<C>::value> struct per_nc;
template <class C> struct per_nc<C, true> { static void* allocate () { return new C; } };
template <class C> struct per_nc<C, false> { static void* allocate () { 
    throw Error("Cannot allocate <mangled: " + String(typeid(C).name()) + "> because it has no nullary constructor.");
} };

 // This class provides the DSL for creating HaccTables.  Every specialization of Haccable
 //  must inherit from this.  Flags are here in case a default
template <class C, uint flags = 0> struct Haccability : GetSet_Builders<C> {
    
    static HaccTable* table;
    static HaccTable* get_table () { return table; }
     // This calls the description code that builds the hacctable.
    static HaccTable* gen_table () {
        table = HaccTable::by_cpptype(typeid(C));
        if (!table) {
            table = new HaccTable(typeid(C));
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
    static void variant (String name, const GetSet1<C>& gs) { get_table()->variants.emplace_back(name, gs); }
};
template <class C, uint flags> HaccTable* Haccability<C, flags>::table = gen_table();

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
#define HCB_COMMA ,
#define HCB_UNQ2(a, b) a##b
#define HCB_UNQ1(a, b) HCB_UNQ2(a, b)
#define HCB_INSTANCE(type) static bool HCB_UNQ1(_HACCABLE_instantiation_, __COUNTER__) = Haccable<type>::table;
#define HCB_BEGIN(type) template <> struct Haccable<type> : hacc::Haccability<type> { static void describe () {
#define HCB_END(type) } }; static bool HCB_UNQ1(_HACCABLE_instantiation_, __COUNTER__) = Haccable<type>::table;
#define HCB_PARAMS(...) __VA_ARGS__
#define HCB_TEMPLATE_BEGIN(params, type) template params struct Haccable<type> : hacc::Haccability<type> { \
    using hacc::Haccability<type>::allocate; \
    using hacc::Haccability<type>::get_id; \
    using hacc::Haccability<type>::find_by_id; \
    using hacc::Haccability<type>::to; \
    using hacc::Haccability<type>::update_from; \
    using hacc::Haccability<type>::delegate; \
    using hacc::Haccability<type>::attr; \
    using hacc::Haccability<type>::elem; \
    using hacc::Haccability<type>::variant; \
    using hacc::Haccability<type>::value_functions; \
    using hacc::Haccability<type>::ref_functions; \
    using hacc::Haccability<type>::ref_function; \
    using hacc::Haccability<type>::assignable; \
    using hacc::Haccability<type>::member; \
    using hacc::Haccability<type>::value_methods; \
    using hacc::Haccability<type>::ref_methods; \
    using hacc::Haccability<type>::ref_method; \
    static void describe () {
#define HCB_TEMPLATE_END(params, type) } };  // Reserved in case we need to do some magic static-var wrangling


#endif
