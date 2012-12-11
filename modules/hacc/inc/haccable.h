
#include "hacc.h"
#include "strings.h"
#include <typeinfo>

namespace hacc {


 // This needs to be declared here so that Haccability<> can access it.
struct HaccTable {
    const std::type_info& cpptype_t;
    String type_s;
    void* (* allocate_p ) ();
    void (* deallocate_p ) (void*);
    Hacc (* to_hacc_p ) (const void*);
     // From_hacc_p is a little dangerous because more of its type has to be erased.
    void* from_hacc_p;
    void (* update_from_hacc_p ) (void*, Hacc);
    void* (* new_from_hacc_p ) (Hacc);
     // TODO
    // String (* generate_id_p ) (void*);
    // void* (* find_by_id_p ) (String);
    // std::vector<Haccribute> attrs;
    // std::vector<Hacclement> elems;

     // Instead of exposing the hash tables, we're wrapping them in these functions
    void reg_cpptype (const std::type_info& t);
    void reg_type (String s);
     // require_* versions throw an exception if the table isn't found.
    static HaccTable* by_cpptype (const std::type_info& t);
    static HaccTable* require_cpptype (const std::type_info& t);
    static HaccTable* by_type (String s);
    static HaccTable* require_type (String s);

     // A constructor template can't actually be called, boo!
    HaccTable (const std::type_info& cpptype_t) :
        cpptype_t(cpptype_t), type_s(""), allocate_p(NULL), deallocate_p(NULL),
        to_hacc_p(NULL), update_from_hacc_p(NULL), new_from_hacc_p(NULL)
    {
        reg_cpptype(cpptype_t);
    }
     // So we have to delegate the fancy stuff elsewhere.
    
};


 // This defines the HACCABLE pseudo-DSL.  It's mixed in via inheritance.
template <class C>
struct Haccability {
    static HaccTable* table;
    static bool create_table (void(* desc )()) {
         // We can't know whether table will be initialized to NULL before or after,
         // but it won't be in the middle of this call, I hope.
        table = new HaccTable(typeid(C));
        desc();
        return true;
    }

    static void type (String s) {
        if (table->type_s.empty())
            table->type_s = s;
        table->reg_type(s);
    }
    static void allocate (C* (* allocate_p )()) {
        table->allocate_p = (void*(*)())allocate_p;
    }
    static void deallocate (void (* deallocate_p )(C*)) {
        table->deallocate_p = (void(*)(void*))deallocate_p;
    }
    static void to (Hacc (* to_hacc_p )(const C&)) {
        table->to_hacc_p = (Hacc(*)(const void*))to_hacc_p;
    }
    static void from (C (* from_hacc_p_ )(Hacc)) {
        table->from_hacc_p = (void*)from_hacc_p_;
    }
    static void update_from (void (* update_from_hacc_p )(C&, Hacc)) {
        table->update_from_hacc_p = (void(*)(void*, Hacc))update_from_hacc_p;
    }
    static void new_from (C* (* new_from_hacc_p )(Hacc)) {
        table->new_from_hacc_p = (void*(*)(Hacc))new_from_hacc_p;
    }

};
template <class C> HaccTable* Haccability<C>::table = NULL;

 // THIS BEGINS the API you use to manipulate haccable objects.

 // Name a type by any means possible
template <class C> String best_type_name () {
    HaccTable* htp = HaccTable::by_cpptype(typeid(C));
    if (htp && !htp->type_s.empty())
        return htp->type_s;
    else 
        return String("<mangled: ") + typeid(C).name() + ">";
}

template <class C> C* allocate () {
    HaccTable* htp = HaccTable::require_cpptype(typeid(C));
    if (htp->allocate_p) {
        return (C*)htp->allocate_p();
    }
    else {
        return new C;
    }
}
template <class C> void deallocate (C* p) {
    HaccTable* htp = HaccTable::require_cpptype(typeid(C));
    if (htp->deallocate_p) {
        htp->deallocate_p((void*)p);
    }
    else {
        delete p;
    }
}
template <class C> Hacc to_hacc (const C& v) {
    HaccTable* htp = HaccTable::require_cpptype(typeid(C));
    if (htp->to_hacc_p) {
        return htp->to_hacc_p((void*)&v);
    }
    throw Error("Tried to call to_hacc on type " + best_type_name<C>() + ", but its Haccable description doesn't have 'to'.");
}
template <class C> C from_hacc (Hacc hacc) {
    HaccTable* htp = HaccTable::require_cpptype(typeid(C));
    if (htp->from_hacc_p) {
        return ((C(*)(Hacc))htp->from_hacc_p)(hacc);
    }
    if (htp->update_from_hacc_p) {
        C r;
        htp->update_from_hacc_p((void*)&r, hacc);
        return r;
    }
    throw Error("Tried to call from_hacc on type " + best_type_name<C>() + ", but its Haccable description doesn't have 'from' or 'update_from'.");
}
template <class C> void update_from_hacc (C* p, Hacc hacc) {
    HaccTable* htp = HaccTable::require_cpptype(typeid(C));
    if (htp->update_from_hacc_p) {
        htp->update_from_hacc_p((void*)p, hacc);
        return;
    }
    if (htp->from_hacc_p) {
        *p = ((C(*)(Hacc))htp->from_hacc_p)(hacc);
        return;
    }
    throw Error("Tried to call update_from_hacc on type " + best_type_name<C>() + ", but its Haccable description doesn't have 'update_from' or 'from'.");
}
template <class C> C* new_from_hacc (Hacc hacc) {
    HaccTable* htp = HaccTable::require_cpptype(typeid(C));
    if (htp->new_from_hacc_p) {
        return (C*)htp->new_from_hacc_p(hacc);
    }
    if (htp->update_from_hacc_p) {
        C* r = allocate<C>();
        htp->update_from_hacc_p((void*)r, hacc);
        return r;
    }
    if (htp->from_hacc_p) {
        C* r = allocate<C>();
        *r = ((C(*)(Hacc))htp->from_hacc_p)(hacc);
        return r;
    }
    throw Error("Tried to call update_from_hacc on type " + best_type_name<C>() + ", but its Haccable description doesn't have 'new_from', 'update_from', or 'from'.");
}

 // Backwards-named aliases
template <class C> Hacc hacc_from (const C& v) {
    return to_hacc<C>(v);
}
template <class C> C hacc_to (Hacc hacc) {
    return from_hacc<C>(hacc);
}
template <class C> C* hacc_to_new (Hacc hacc) {
    return new_from_hacc<C>(hacc);
}

 // Directly to and from strings.
template <class C> String to_string (const C& v) {
    return hacc_to_string(to_hacc<C>(v));
}
template <class C> C from_string (String s) {
    return from_hacc<C>(hacc_from_string(s));
}
template <class C> void update_from_string (C* p, String s) {
    update_from_hacc<C>(p, hacc_from_string(s));
}
template <class C> C string_to (String s) {
    return from_string<C>(s);
}
template <class C> String string_from (const C& v) {
    return to_string<C>(v);
}
template <class C> C* new_from_string (String s) {
    return new_from_hacc<C>(hacc_from_string(s));
}
template <class C> C* string_to_new (String s) {
    return new_from_string<C>(s);
}

}

 // Finally, this is how you make a haccable definition.
#define HACCABLE(C, ...) \
    namespace { \
        template <> struct HACCABLE_Definition<C> : hacc::Haccability<C> { \
            static void describe () __VA_ARGS__ \
            static bool created; \
        }; \
        bool HACCABLE_Definition<C>::created = create_table(describe); \
    } \

 // example:
 // HACCABLE(mything, {
 //     type("mything");
 //     to([](mything x){ return Hacc(x.as_integer); });
 //     from([](Hacc h){ return mything(h.get_integer); });
 // })

 // Only allow one definition per type at compile-time per compilation unit.
namespace { template <class C> struct HACCABLE_Definition; }

