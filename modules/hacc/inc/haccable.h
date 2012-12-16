#ifndef HAVE_HACC_HACCABLE_H
#define HAVE_HACC_HACCABLE_H

#include "hacc.h"

namespace hacc {
struct Haccer;
}

namespace hacc {

 // Okay, the mechanics of how haccabilities get registered is a little difficult to follow.

 // A custom Haccable<C> should from Haccability<C>
 //  If it does, Haccability<C> is instantiated if Haccable<C> is instantiated.
 //  When Haccability<C> is instantiated, it registers a HaccTable under typeid(C).
 //  It sets its static variable table to that.  Haccable<C> inherits it.
 // The default Haccable<C> does not inherit from Haccability<C>.
 //  When instantiated, it (lazily) looks for a HaccTable registered under typeid(C).
 //  It sets its static variable table to that.

 // When you want to hacc a type, the haccing procedures will look for Haccable<C>::table.
 //  This will instantiate Haccable<C>, causing one of the above two scenarios to happen
 //   at init-time.
 //  In either case, Haccable<C> ends up with a ::table.

 // When you define a Haccable<C>, the behaviors you specify trickle down via virtual methods
 // through Haccability<C> to HaccTable.

struct HaccTable {
    const std::type_info& cpptype;
    virtual void info () = 0;
    virtual void describe (void*, void*) = 0;
    virtual String haccid (void*) = 0;
    virtual void* g_find_by_haccid (String) = 0;

    bool infoized = false;
    void infoize ();

    String _hacctype;
    String get_hacctype ();
    void hacctype (String s);

    std::vector<HaccTable*> bases;
    void base (HaccTable*);  // Adds a base.
    bool has_base (HaccTable* b);
    
    uint32 _flags = 0;
    uint32 get_flags ();
    enum {
        ADVERTISE_ID,
        ADVERTISE_TYPE,
    };
    void advertise_id ();
    void advertise_type ();

     // This calls new_from_hacc.
    void* manifest ();

     // Creation
    static HaccTable* by_cpptype (const std::type_info&);
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
#define HCB_BEGIN(type) template <> struct Haccable<type> : hacc::Haccability<type>
#define HCB_END(type) ; HCB_INSTANCE(type)
#define HCB_PARAMS(...) __VA_ARGS__
#define HCB_TEMPLATE_BEGIN(params, type) template params struct Haccable<type> : hacc::Haccability<type>
#define HCB_TEMPLATE_END(params, type) ;  // Reserved in case we need to do some magic static-var wrangling

namespace hacc {

template <class C, uint flags = 0> struct Haccability : HaccTable {
    virtual void info () { }
    virtual void describe (Haccer&, C&) {
        throw Error ("The Haccable for <mangled: " + typeid(C) + "> has no describe.");
    }
    void describe (void* h, void* it) { describe(*(Haccer*)h, *(C*)it); }
    virtual String haccid (const C& v) { return ""; }
    String haccid (void* p) { return haccid(*(const C*)p); }
    virtual C* find_by_haccid (String s) { return null; }
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
        t->describe((void*)&h, (void*)&it);
    }

    template <class C> Hacc to_hacc (C& v) {
        Haccer::Writer w;
        run_description(w, v);
        return w.hacc;
    }

    template <class C> Hacc hacc_from (C& v) {
        return to_hacc<C>(v);
    }
    template <class C> Hacc hacc_from (C v) {
        return to_hacc<C>(v);
    }

    template <class C> void update_from_hacc (C& v, Hacc h) {
        Haccer::Validator validator (h);
        run_description(validator, v);
        validator.finish();
        Haccer::Reader reader (h);
        run_description(reader, v);
        Haccer::Finisher finisher (h);
        run_description(finisher, v);
    }
    template <class C> C from_hacc (Hacc h) {
        C r;
        update_from_hacc(r, h);
        return r;
    }
    template <class C> C hacc_to (Hacc h) {
        C r;
        update_from_hacc(r, h);
        return r;
    }
     // Auto-deallocate a pointer if an exception happens.
    template <class C>
    struct Bomb {
        C* p;
        Bomb (C* p) :p(p) { }
        ~Bomb () { if (p) delete p; }
        void defuse () { p = null; }
    };
    template <class C> C* new_from_hacc (Hacc h) {
        C* r = new C;
        Bomb<C> b (r);
        update_from_hacc(*r, h);
        b.defuse();
        return r;
    }
    template <class C> C* hacc_to_new (Hacc h) {
        return new_from_hacc<C>(h);
    }

}

#endif
