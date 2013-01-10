#ifndef HAVE_HACC_HACCABLE_H
#define HAVE_HACC_HACCABLE_H

#include <unordered_map>
#include "hacc.h"

namespace hacc {
struct Haccer;
typedef std::unordered_map<String, void*> ID_Map;
}

namespace hacc {

struct HaccTable {
    const std::type_info& cpptype;
     // This defines a bunch of things.  Like, a lot.
    virtual void info () = 0;
     // Nullary allocator.
    virtual void* g_allocate () = 0;
     // Working with ids
    virtual String get_id (void*) = 0;
    virtual void* g_find_by_id (String) = 0;

     // Update_from is the only internal way of reading from hacc
    virtual void update_from_hacc (void*, Hacc) = 0;
    virtual Hacc&& to_hacc (void*) = 0;

    bool infoized = false;
    void infoize ();

     // This calls new_from_hacc.
    void* manifest ();

     // Creation
    static HaccTable* by_cpptype (const std::type_info&);
    static HaccTable* by_hacctype (String);
    HaccTable () : cpptype(typeid(null)) { }
    HaccTable (const std::type_info& t);
};

}  // Haccable is outside the namespace for specialization convenience
template <class C> struct Haccable {
    static hacc::HaccTable* get_table () {
        static hacc::HaccTable* table = hacc::HaccTable::by_cpptype(typeid(C));
        return table;
    }
};

 // I guess we need to use some macros after all.
#define HCB_UNQ2(a, b) a##b
#define HCB_UNQ1(a, b) HCB_UNQ2(a, b)
#define HCB_INSTANCE(type) static bool HCB_UNQ1(_HACCABLE_instantiation_, __COUNTER__) = Haccable<type>::table;
#define HCB_W_BASE(type, base) template <> struct Haccable<type> : base
#define HCB_BEGIN(type) HCB_W_BASE(type, hacc::Haccability<type>)
#define HCB_END(type) ; HCB_INSTANCE(type)
#define HCB_PARAMS(...) __VA_ARGS__
#define HCB_TEMPLATE_W_BASE(params, type, base) template params struct Haccable<type> : base
#define HCB_TEMPLATE_BEGIN(params, type) HCB_TEMPLATE_W_BASE(params, type, hacc::Haccability<type>)
#define HCB_TEMPLATE_END(params, type) ;  // Reserved in case we need to do some magic static-var wrangling

namespace hacc {

template <class C, uint flags = 0> struct Haccability : HaccTable {
     // Okay, here are a bunch of function pointer thingies.
    GetSet<C> id;  // The inner type must be string.
     // Leave it to something else.
     // If the inner type of this is Hacc, it defines a direct transition.
    GetSet<C> delegate;
     // Defined by attributes with names
    Map<GetSet<C>> attrs;
     // Defined by a fixed number of elements
    VArray<GetSet<C>> elems;
     // Variants with names specific to this interface
     // Note that this will only be used if following a pointer.
    Map<GetSet<C>> variants;
    
     // Implement all the 
    virtual void info () = 0;
    String get_id (void* p) { return id.; }
    void* g_find_by_haccid (String s) { return (void*)find_by_haccid(s); }

    template <class B> void base () {
        HaccTable* t = HaccTable::by_cpptype(typeid(B));
        if (t) base(t);
        else throw Error("Base type <mangled: " + String(typeid(B).name()) + "> is not haccable.");
    }

     // Bookkeeping
    static HaccTable* table;
    static HaccTable* get_table () { return table; }
    static HaccTable* gen_table () {
        HaccTable* r = HaccTable::by_cpptype(typeid(C));
        if (!r) r = new Haccable<C>;
        return r;
    }
    Haccability () : HaccTable(typeid(C)) { }
};
template <class C, uint flags> HaccTable* Haccability<C, flags>::table = gen_table();

template <class C> void run_description(Haccer& h, C& it);

}


#include "haccers.h"



namespace hacc {

    template <class C>
    void run_description (Haccer& h, C& it) {
        HaccTable* t = Haccable<C>::get_table();
        if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
        t->describe(h, (void*)&it);
    }

    template <class C> Hacc to_hacc (const C& v) {
        HaccTable* t = Haccable<C>::get_table();
        if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
        Haccer::Writer w (t);
        run_description(w, const_cast<C&>(v));
        return std::move(w.hacc);
    }

    template <class C> Hacc hacc_from (const C& v) {
        return to_hacc(v);
    }
    
    void g_update_from_hacc (HaccTable* t, void* p, const Hacc& h);
    template <class C> void update_from_hacc (C& v, const Hacc& h) {
        HaccTable* t = Haccable<C>::get_table();
        if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
        g_update_from_hacc(t, (void*)&v, h);
    }
    template <class C> C from_hacc (const Hacc& h) {
        C r;
        update_from_hacc(r, h);
        return r;
    }
    template <class C> C hacc_to (const Hacc& h) {
        C r;
        update_from_hacc(r, h);
        return r;
    }
     // Auto-deallocate a pointer if an exception happens.
    namespace { template <class C> struct Bomb {
        C* p;
        Bomb (C* p) :p(p) { }
        ~Bomb () { if (p) delete p; }
        void defuse () { p = null; }
    }; }
    template <class C> C* new_from_hacc (const Hacc& h) {
        C* r = new C;
        Bomb<C> b (r);
        update_from_hacc(*r, h);
        b.defuse();
        return r;
    }
    template <class C> C* hacc_to_new (const Hacc& h) {
        return new_from_hacc<C>(h);
    }

    template <class C> String hacctype () {
        HaccTable* t = Haccable<C>::get_table();
        if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
        return t->get_hacctype();
    }

    template <class C> String haccid (const C& v) {
        HaccTable* t = Haccable<C>::get_table();
        if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
        return t->haccid((void*)&v);
    }
    template <class C> C* find_by_haccid (String id) {
        HaccTable* t = Haccable<C>::get_table();
        if (!t) throw Error("No Haccable was defined for type <mangled: " + String(typeid(C).name()) + ">.");
        return (C*)t->g_find_by_haccid(id);
    }
    template <class C> C* find_by_id (String id) { return find_by_haccid<C>(id); }

}

#endif
