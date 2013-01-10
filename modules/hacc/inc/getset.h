#ifndef HAVE_HACC_GETSET_H
#define HAVE_HACC_GETSET_H

#include <functional>

namespace hacc {

    template <class X>
    struct GetSet {
        const std::type_info& mtype;

        typedef Func<void (const X&, void (*)(void*))> Get_CPS;
        typedef Func<void (X&, void*)> Set;
        Get_CPS get_cps;
        Set set;
    };

//    template <class X, class M>
//    GetSet<X> GS (Func<void (const X&, void (*)(const M&))> g, Func<void (X&, const M&)> s) {
//        return GetSet<X>{typeid(M), *(GetSet<X>::Get_CPS*)&g, *(GetSet<X>::Set*)&s};
//    }

     // Most of the common ways of providing get-set behavior.
     // Member pointer
    template <class X, class M>
    GetSet<X> GS (M X::* p) {
        return GetSet<X>{
            typeid(M),
            [](const X& x, void (* c )(void*)){ ((void(*)(const M&))c)(x.*p); },
            [](X& x, void* p){ x.*p = *(M*)p; }
        };
    }
     // Functions with raw values
    template <class X, class M>
    GetSet<X> GS (Func<M (const X&)> g, Func<void (X&, M)> s) {
        return GetSet<X>{
            typeid(M),
            [](const X& x, void (* c )(void*)){ ((void(*)(const M&))c)(g(x)); },
            [](X& x, void* p){ s(x, *(M*)p); }
        };
    }
     // Functions with references
    template <class X, class M>
    GetSet<X> GS (Func<const M& (const X&)> g, Func<void (X&, const M&)> s) {
        return GetSet<X>{
            typeid(M),
            [](const X& x, void (* c )(void*)){ ((void(*)(const M&))c)(g(x)); },
            [](X& x, void* p){ s(x, *(M*)p); }
        };
    }
     // Method pointers with raw values
    template <class X, class M>
    GetSet<X> GS (M (X::* g )()const, void (X::* s )(M)) {
        return GetSet<X>{
            typeid(M),
            [](const X& x, void (* c )(void*)){ ((void(*)(const M&))c)(x.*g()); },
            [](X& x, void* p){ x.*s(*(M*)p); }
        };
    }
     // Method pointers with references
    template <class X, class M>
    GetSet<X> GS (const M& (X::* g )()const, void (X::* s )(const M&)) {
        return GetSet<X>{
            typeid(M),
            [](const X& x, void (* c )(void*)){ ((void(*)(const M&))c)(x.*g()); },
            [](X& x, void* p){ x.*s(*(M*)p); }
        };
    }
     // Coercion and assignment
    template <class X, class M>
    GetSet<X> GS () {
        return GetSet<X>{
            typeid(M),
            [](const X& x, void (* b )(void*)){ ((void(*)(const M&))c)(x); },
            [](X& x, void* p){ x = *(M*)p; }
        }
    }

}

#endif
