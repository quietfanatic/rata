#ifndef HAVE_HACC_HACCABLE_H
#define HAVE_HACC_HACCABLE_H


#include <typeinfo>
#include <string.h>

#include "hacc.h"

namespace hacc { struct HaccTable; }

 // Base implementation of the thing that implements HACCABLE definitions.
 // The HACCABLE macro creates a specialization of this.
namespace {
    template <class C> struct HACCABLE_Definition {
         // If this is not null, it means that the haccable definition for this type
         // has been defined in this compilation unit, and hasn't been overridden
         // by a definition in a different compilation unit.
        static hacc::HaccTable* table;
    };
    template <class C> hacc::HaccTable* HACCABLE_Definition<C>::table = hacc::null;
}

namespace hacc {

template <class T>
using F = T;  // Makes function pointers nicer.

struct Haccribute {
    const std::type_info& cpptype;
    String name;
    F<void* (void*)>* ref;
    Haccribute (const std::type_info& cpptype, String name, F<void* (void*)>* ref) :
        cpptype(cpptype), name(name), ref(ref)
    { }
};

 // Make decisions based on whether a type has a nullary constructor
template <class C, bool has_nc> struct _constructibility;
template <class C> struct _constructibility<C, true> {
    static inline C* allocate () { return new C; }
    static inline void construct (C& p) { new (&p) C; }
};
template <class C> struct _constructibility<C, false> {
    static constexpr F<C* ()>* allocate { null };
    static constexpr F<void (C&)>* construct { null };
};
template <class C>
using constructibility = _constructibility<C, std::is_constructible<C>::value>;


 /// This needs to be declared here so that Haccability<> can access it.
struct HaccTable {
    const std::type_info& cpptype;
    size_t cppsize;
    String hacctype = "";
    F<String ()>* calc_hacctype { null };  // Using = null triggers a compiler bug.
    F<String (void*)>* haccid { null };
    F<void* (String)>* find_by_haccid { null };
    write_options options = write_options(0);
    F<void* ()>* allocate { null };
    F<void (void*)>* deallocate { null };
    F<void (void*)>* construct { null };
    F<Hacc (void*)>* to_hacc { null };
    void* from_hacc { null };  // Unboxedness screws things up.
    F<void (void*, Hacc)>* update_from_hacc { null };
    F<void* (Hacc)>* new_from_hacc { null };
    VArray<Haccribute> attrs;
     // TODO
    //VArray<Hacclement> elems;

     // Accessors that do a little bit of magic.
    String get_hacctype ();
    String do_haccid (void* p);
    void* do_find_by_haccid (String id);
    void* do_allocate ();
    void do_deallocate (void*);
    void do_construct (void*);
    Hacc do_to_hacc (void*, write_options);
    void* get_from_hacc ();
    void do_update_from_hacc (void*, Hacc);
    void* do_new_from_hacc (Hacc);

     // Instead of exposing the hash tables, we're wrapping them in these functions
    static HaccTable* gen (const std::type_info& cpptype, size_t cppsize);
    void add_hacctype (String s);
    void finalize ();
     // require_* versions throw an exception if the table isn't found.
    static HaccTable* by_cpptype (const std::type_info& t);
    static HaccTable* require_cpptype (const std::type_info& t);
    static HaccTable* by_hacctype (String s);
    static HaccTable* require_hacctype (String s);

    HaccTable (const std::type_info& cpptype, size_t cppsize) : cpptype(cpptype), cppsize(cppsize) { }
    
};


 /// This defines the HACCABLE pseudo-DSL.  It's mixed in via inheritance.
template <class C> struct Haccability {
    static HaccTable* table;
    static HaccTable* describe (F<void (Haccability<C>)>* desc) {
        table = HaccTable::gen(typeid(C), sizeof(C));
        if (table) {
            desc(Haccability<C>());
            table->finalize();
        }
        return table;
    }

    static void hacctype (String s) { table->add_hacctype(s); }
    static void hacctype (F<String ()>* p) { table->calc_hacctype = p; }
    static void haccid (F<String (const C&)>* p) { table->haccid = (F<String (void*)>*)p; }
    static void find_by_haccid (F<C* (String)>* p) { table->find_by_haccid = (F<void* (String)>*)p; }
    static void options (write_options opts) { table->options = opts; }
    static void allocate (F<C* ()>* p) { table->allocate = (F<void* ()>*)p; }
    static void deallocate (F<void (C*)>* p) { table->deallocate = (F<void (void*)>*)p; }
    static void construct (F<void (C&)>* p) { table->construct = (F<void (void*)>*)p; }
    static void to_hacc (F<Hacc (const C&)>* p) { table->to_hacc = (F<Hacc (void*)>*)p; }
    static void hacc_from (F<Hacc (const C&)>* p) { table->to_hacc = (F<Hacc(void*)>)p; }
    static void from_hacc (F<C (Hacc)>* p) { table->from_hacc = (void*)p; }
    static void hacc_to (F<C (Hacc)>* p) { table->from_hacc = (void*)p; }
    static void update_from_hacc (F<void (C&, Hacc)>* p) { table->update_from_hacc = (F<void (void*, Hacc)>*)p; }
    static void new_from_hacc (F<C* (Hacc)>* p) { table->new_from_hacc = (F<void* (Hacc)>*)p; }
    template <class M>
    static void attr (String name, F<M& (C&)>* ref) {
        table->attrs.push_back(Haccribute(typeid(M), name, (F<void* (void*)>*)ref));
    }
     // It's easy to hacc types that can be converted to and from an atomic type.
     // Just use these functions, e.g. like_integer();
     // Make sure the conversion to the atomic type is 'const' on 'this'.
#define HACCABLE_BUILD_LIKE_FUNCTION(lc, uc) \
    static void like_##lc () { \
        to_hacc([](const C& x)->Hacc{ return Hacc(static_cast<uc>(x)); }); \
        from_hacc([](Hacc h)->C{ return static_cast<C>(h.get_##lc()); }); \
    }
    HACCABLE_BUILD_LIKE_FUNCTION(null, Null)
    HACCABLE_BUILD_LIKE_FUNCTION(bool, Bool)
    HACCABLE_BUILD_LIKE_FUNCTION(integer, Integer)
    HACCABLE_BUILD_LIKE_FUNCTION(float, Float)
    HACCABLE_BUILD_LIKE_FUNCTION(double, Double)
    HACCABLE_BUILD_LIKE_FUNCTION(string, String)
};
 // Dummy initialization.
template <class C> HaccTable* Haccability<C>::table = Haccability<C>::table;

 // The primary purpose of this is not actually to get a HaccTable.  It's
 //  to force instantiation of the HACCABLE_Definition.  As a bonus, if
 //  the definition has been given in this compilation unit, it'll skip
 //  one hashtable lookup.
template <class C> HaccTable* get_HaccTable () {
    if (!HACCABLE_Definition<C>::table)
        HACCABLE_Definition<C>::table = HaccTable::by_cpptype(typeid(C));
    return HACCABLE_Definition<C>::table;
}
template <class C> HaccTable* require_HaccTable () {
    if (!HACCABLE_Definition<C>::table)
        HACCABLE_Definition<C>::table = HaccTable::require_cpptype(typeid(C));
    return HACCABLE_Definition<C>::table;
}

 // Name a type by any means possible
String best_type_name (HaccTable*);
String best_type_name (const std::type_info&);
template <class C> String best_type_name () {
    return best_type_name(typeid(C));
}

template <class C> String best_type_name (const C& v) {
    return best_type_name<C>();
}

 // If you don't provide an interpretation for the ids of a particular type,
 // its ids will be based on the memory addresses of its values
String address_to_id (void* addr);
void* id_to_address (String id);


 // THIS BEGINS the API you use to manipulate haccable objects.


template <class C> bool has_hacctype () {
    HaccTable* htp = get_HaccTable<C>();
    return htp && (htp->calc_hacctype || !htp->hacctype.empty());
}
template <class C> bool has_hacctype (const C& v) {
    return has_hacctype<C>();
}

template <class C> String hacctype () {
    return require_HaccTable<C>()->get_hacctype();
}
template <class C> String hacctype (const C& v) {
    return hacctype<C>();
}

template <class C> String haccid (const C& v) {
    return require_HaccTable<C>()->do_haccid((void*)&v);
}
template <class C> C* find_by_haccid (String id) {
    return (C*)require_HaccTable<C>()->do_find_by_haccid(id);
}

template <class C> C* allocate () {
    return (C*)require_HaccTable<C>()->do_allocate();
}
template <class C> void deallocate (C* p) {
    return require_HaccTable<C>()->do_deallocate((void*)p);
}
template <class C> Hacc to_hacc (const C& v, write_options opts = write_options(0)) {
    return require_HaccTable<C>()->do_to_hacc((void*)&v, opts);
}
template <class C> C from_hacc (Hacc hacc) {
    return ((F<C(Hacc)>*)require_HaccTable<C>()->get_from_hacc())(hacc);
}
template <class C> void update_from_hacc (C& p, Hacc hacc) {
    return require_HaccTable<C>()->do_update_from_hacc((void*)&p, hacc);
}
template <class C> C* new_from_hacc (Hacc hacc) {
    return (C*)require_HaccTable<C>()->do_new_from_hacc(hacc);
}

 // Backwards-named aliases
template <class C> Hacc hacc_from (const C& v, write_options opts = write_options(0)) {
    return to_hacc<C>(v, opts);
}
template <class C> C hacc_to (Hacc hacc) {
    return from_hacc<C>(hacc);
}
template <class C> C* hacc_to_new (Hacc hacc) {
    return new_from_hacc<C>(hacc);
}


}

 // Manually force instantiation of an instance
#define HACCABLE_INSTANCE_CTR2(C, ctr) static bool HACCABLE_instance_##ctr = HACCABLE_Definition<C>::table;
#define HACCABLE_INSTANCE_CTR1(C, ctr) HACCABLE_INSTANCE_CTR2(C, ctr)
#define HACCABLE_INSTANCE(C) HACCABLE_INSTANCE_CTR1(C, __COUNTER__)

 // Finally, this is how you make a haccable definition.
#define HACCABLE_BEGIN(C) \
namespace { \
    template <> struct HACCABLE_Definition<C> { \
        static void desc (hacc::Haccability<C> d)
#define HACCABLE_END(C) \
        static hacc::HaccTable* table; \
    }; \
    hacc::HaccTable* HACCABLE_Definition<C>::table = hacc::Haccability<C>::describe(desc); \
    HACCABLE_INSTANCE(C) \
}
#define HACCABLE(C, ...) HACCABLE_BEGIN(C) __VA_ARGS__ HACCABLE_END(C)
 // Apparently looking up names in a parent class that depends on a template
 // parameter to a template specialization template is too much for C++ to handle.
 // So we need to manually import all the DSL names in Haccability<C>.
#define HACCABLE_TEMPLATE_BEGIN(params, C) \
namespace { \
    template params struct HACCABLE_Definition<C> { \
        static void desc (hacc::Haccability<C> d)
#define HACCABLE_TEMPLATE_END(params, C) \
        static hacc::HaccTable* table; \
    }; \
    template params hacc::HaccTable* HACCABLE_Definition<C>::table = hacc::Haccability<C>::describe(desc); \
}
#define HACCABLE_TEMPLATE(params, C, ...) HACCABLE_TEMPLATE_BEGIN(params, C) __VA_ARGS__ HACCABLE_TEMPLATE_END(params, C)

 // To allow commas in type names or template parameter lists given to macros.
#define GRP(...) __VA_ARGS__

 // example:
 // HACCABLE(mything, {
 //     d.hacctype("mything");
 //     d.to_hacc([](mything x){ return Hacc(x.as_integer); });
 //     d.from_hacc([](Hacc h){ return mything(h.get_integer); });
 // })





 // Wow, I gotta say, it's crazy that all this stuff actually works.

#endif
