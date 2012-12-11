
#include "hacc.h"
#include "strings.h"
#include <typeinfo>



 // Base implementation of the thing that implements HACCABLE definitions.
 // The HACCABLE macro creates a specialization of this.
namespace {
    template <class C> struct HACCABLE_Definition {
         // If this is true, it means that the haccable definition for this type
         // has been defined in this compilation unit, and hasn't been overridden
         // by a definition in a different compilation unit.
        static bool registered;
    };
    template <class C> bool HACCABLE_Definition<C>::registered = false;
}

namespace hacc {

 // This needs to be declared here so that Haccability<> can access it.
struct HaccTable {
    const std::type_info& cpptype_t;
    String type_s;
    String (* get_type_p ) (void*);
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
         // Prevent duplicate definitions.
        if (HaccTable::by_cpptype(typeid(C))) return false;
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
    static void get_type (String (* get_type_p )(const C&)) {
        table->get_type_p = (String(*)(void*))get_type_p;
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
     // It's easy to hacc types that can be converted to and from an atomic type.
     // Just use these functions, e.g. like_integer();
     // Make sure the conversion to the atomic type is 'const' on 'this'.
#define HACCABLE_BUILD_LIKE_FUNCTION(lc, uc) \
    static void like_##lc () { \
        to([](const C& x)->Hacc{ return Hacc(static_cast<uc>(x)); }); \
        from([](Hacc h)->C{ return static_cast<C>(h.get_##lc()); }); \
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
template <class C> HaccTable* require_HaccTable () {
    if (HACCABLE_Definition<C>::registered)
        return Haccability<C>::table;
    else return HaccTable::require_cpptype(typeid(C));
}


 // THIS BEGINS the API you use to manipulate haccable objects.

 // Name a type by any means possible
template <class C> String best_type_name () {
    HaccTable* htp = HACCABLE_Definition<C>::registered
        ? Haccability<C>::table
        : HaccTable::by_cpptype(typeid(C));
    if (htp && !htp->type_s.empty())
        return htp->type_s;
    return String("<mangled: ") + typeid(C).name() + ">";
}

template <class C> String best_type_name (const C& v) {
    HaccTable* htp = HACCABLE_Definition<C>::registered
        ? Haccability<C>::table
        : HaccTable::by_cpptype(typeid(C));
    if (htp) {
        if (htp->get_type_p)
            return ((String(*)(const C&))htp->get_type_p)(v);
        if (!htp->type_s.empty())
            return htp->type_s;
    }
    return String("<mangled: ") + typeid(C).name() + ">";
}

template <class C> bool has_type () {
    HaccTable* htp = HACCABLE_Definition<C>::registered
        ? Haccability<C>::table
        : HaccTable::by_cpptype(typeid(C));
    return htp && !htp->type_s.empty();
}
template <class C> bool has_type (const C& v) {
    HaccTable* htp = HACCABLE_Definition<C>::registered
        ? Haccability<C>::table
        : HaccTable::by_cpptype(typeid(C));
    return htp && (htp->get_type_p || !htp->type_s.empty());
}

template <class C> String get_type () {
    HaccTable* htp = require_HaccTable<C>();
    if (!htp->type_s.empty())
        return htp->type_s;
    if (htp->get_type_p)
        throw Error("Tried to get type of <mangled: " + String(typeid(C).name()) + ">, but no 'type' was given in its haccable description.  However, the haccable description did contain 'get_type'; to use that you must call hacc::get_type on a particular instance of this type.");
    throw Error("Tried to get type of <mangled: " + String(typeid(C).name()) + ">, but no 'type' was given in its haccable description.");
}
template <class C> String get_type (const C& v) {
    HaccTable* htp = require_HaccTable<C>();
    if (htp->get_type_p)
        return ((String(*)(const C&))htp->get_type_p)(v);
    if (!htp->type_s.empty())
        return htp->type_s;
    throw Error("Tried to get type of an instance of <mangled: " + String(typeid(C).name()) + ">, but its haccable description doesn't have 'type' or 'get_type'.");
}

template <class C> C* allocate () {
    HaccTable* htp = require_HaccTable<C>();
    if (htp->allocate_p) {
        return (C*)htp->allocate_p();
    }
    else {
        return new C;
    }
}
template <class C> void deallocate (C* p) {
    HaccTable* htp = require_HaccTable<C>();
    if (htp->deallocate_p) {
        htp->deallocate_p((void*)p);
    }
    else {
        delete p;
    }
}
template <class C> Hacc to_hacc (const C& v) {
    HaccTable* htp = require_HaccTable<C>();
    if (htp->to_hacc_p) {
        return htp->to_hacc_p((void*)&v);
    }
    throw Error("Tried to call to_hacc on type " + best_type_name<C>() + ", but its Haccable description doesn't have 'to'.");
}
template <class C> C from_hacc (Hacc hacc) {
    HaccTable* htp = require_HaccTable<C>();
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
    HaccTable* htp = require_HaccTable<C>();
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
    HaccTable* htp = require_HaccTable<C>();
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

 // Manually force instantiation of an instance
#define HACCABLE_INSTANCE_CTR2(C, ctr) static bool HACCABLE_instance_##ctr = HACCABLE_Definition<C>::registered;
#define HACCABLE_INSTANCE_CTR1(C, ctr) HACCABLE_INSTANCE_CTR2(C, ctr)
#define HACCABLE_INSTANCE(C) HACCABLE_INSTANCE_CTR1(C, __COUNTER__)

 // Finally, this is how you make a haccable definition.
#define HACCABLE_BEGIN(C) \
namespace { \
    template <> struct HACCABLE_Definition<C> : hacc::Haccability<C> { \
        static void describe ()
#define HACCABLE_END(C) \
        static bool registered; \
    }; \
    bool HACCABLE_Definition<C>::registered = hacc::Haccability<C>::create_table(describe); \
    HACCABLE_INSTANCE(C) \
}
#define HACCABLE(C, ...) HACCABLE_BEGIN(C) __VA_ARGS__ HACCABLE_END(C)
 // Apparently looking up names in a parent class that depends on a template
 // parameter to a template specialization template is too much for C++ to handle.
 // So we need to manually import all the DSL names in Haccability<C>.
#define HACCABLE_TEMPLATE_BEGIN(params, C) \
namespace { \
    template params struct HACCABLE_Definition<C> : hacc::Haccability<C> { \
        typedef hacc::Haccability<C> ht; \
        using ht::type; \
        using ht::get_type; \
        using ht::allocate; \
        using ht::deallocate; \
        using ht::to; \
        using ht::from; \
        using ht::update_from; \
        using ht::new_from; \
        using ht::like_null; \
        using ht::like_bool; \
        using ht::like_integer; \
        using ht::like_float; \
        using ht::like_double; \
        using ht::like_string; \
        static void describe ()
#define HACCABLE_TEMPLATE_END(params, C) \
        static bool registered; \
    }; \
    template params bool HACCABLE_Definition<C>::registered = hacc::Haccability<C>::create_table(describe); \
}
#define HACCABLE_TEMPLATE(params, C, ...) HACCABLE_TEMPLATE_BEGIN(params, C) __VA_ARGS__ HACCABLE_TEMPLATE_END(params, C)

 // To allow commas in type names or template parameter lists given to macros.
#define GRP(...) __VA_ARGS__

 // example:
 // HACCABLE(mything, {
 //     type("mything");
 //     to([](mything x){ return Hacc(x.as_integer); });
 //     from([](Hacc h){ return mything(h.get_integer); });
 // })





 // Wow, I gotta say, it's crazy that all this stuff actually works.

