#ifndef HAVE_HACC_GETSET_H
#define HAVE_HACC_GETSET_H

 // This file provides black magic for encapsulation.
 //
 // A GetSet is the description of an attribute-like-thing of an object.  One can be created from:
 //   A pointer-to-member
 //   Getter and setter (methods|functions) with pass-by-(value|reference) semantics
 //   A matching pair of coercions or operator= methods
 // Soon we may allow creation from:
 //   A (method|function) returning a read-writable reference
 //
 // TODO: A Default simply wraps default value declarations in a lambda for type erasure.

#include <functional>
template <class F> using Func = std::function<F>;

namespace hacc {

    struct GetSet {
        const std::type_info* xtype;
        const std::type_info* mtype;
        
         // In order to minimize copying of temporaries, get and set have to be
         // higher-order functions.  Using std::function imposes some overhead, but
         // not nearly as much as could potentially be imposed by having to copy
         // a large piece of data when we could update in place instead.
        
         // A getter takes an X and a function that will process the gotten value.
        typedef Func<void (void*, Func<void (void*)>)> Get;
         // A setter takes an X and a function that will update the settable value through a pointer.
        typedef Func<void (void*, Func<void (void*)>)> Set;
        Get get;
        Set set;
        GetSet () : xtype(null), mtype(null) { }
        GetSet (const std::type_info& xtype, const std::type_info& mtype, Get get, Set set) :
            xtype(&xtype), mtype(&mtype), get(get), set(set)
        { }
    };

//    template <class X, class M>
//    GetSet<X> GS (Func<void (const X&, void (*)(const M&))> g, Func<void (X&, const M&)> s) {
//        return GetSet<X>{typeid(M), *(GetSet<X>::Get_CPS*)&g, *(GetSet<X>::Set*)&s};
//    }

     // Most of the common ways of providing get-set behavior.
     // Member pointer
    template <class X, class M>
    GetSet GS (M X::* p) {
        return GetSet(
            typeid(X),
            typeid(M),
            [p](void* x, Func<void (void*)> c){ const M& tmp = ((X*)x)->*p; c((void*)&tmp); },
            [p](void* x, Func<void (void*)> c){ M& tmp = ((X*)x)->*p; c(&tmp); }
        );
    }
     // Functions with raw values
    template <class X, class M>
    GetSet GS (Func<M (const X&)> g, Func<void (X&, M)> s) {
        return GetSet(
            typeid(X),
            typeid(M),
            [g, s](void* x, Func<void (void*)> c){ const X& tmp = g(*(const X*)x); c(&tmp); },
            [g, s](void* x, Func<void (void*)> c){ M tmp; c(&tmp); s(*(X*)x, tmp); }
        );
    }
     // Functions with references
    template <class X, class M>
    GetSet GS (Func<const M& (const X&)> g, Func<void (X&, const M&)> s) {
        return GetSet(
            typeid(X),
            typeid(M),
            [g, s](void* x, Func<void (void*)> c){ c((void*)&g(*(const X*)x)); },
            [g, s](void* x, Func<void (void*)> c){ M tmp; c(&tmp); s(*(X*)x, tmp); }
        );
    }
     // Method pointers with raw values
    template <class X, class M>
    GetSet GS (M (X::* g )()const, void (X::* s )(M)) {
        return GetSet(
            typeid(X),
            typeid(M),
            [g, s](void* x, Func<void (void*)> c){ const M& tmp = ((X*)x)->*g(); c((void*)&tmp); },
            [g, s](void* x, Func<void (void*)> c){ M tmp; c(tmp); ((X*)x)->*s(tmp); }
        );
    }
     // Method pointers with references
    template <class X, class M>
    GetSet GS (const M& (X::* g )()const, void (X::* s )(const M&)) {
        return GetSet(
            typeid(X),
            typeid(M),
            [g, s](void* x, Func<void (void*)> c){ c((void*)&((X*)x)->*g()); },
            [g, s](void* x, Func<void (void*)> c){ M tmp; c(tmp); ((X*)x)->*s(tmp); }
        );
    }
     // Assignability
    template <class X, class M>
    GetSet GS () {
        return GetSet(
            typeid(X),
            typeid(M),
            [](void* x, Func<void (void*)> c){ M tmp = *(X*)x; c(&tmp); },
            [](void* x, Func<void (void*)> c){ M tmp; c(&tmp); *(X*)x = tmp; }
        );
    }

}

#endif
