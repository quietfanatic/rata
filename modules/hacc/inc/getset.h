#ifndef HAVE_HACC_GETSET_H
#define HAVE_HACC_GETSET_H

 // This file provides three classes for encapsulating OO magic attributes.
 // GetSet2, GetSet1, and GetSet0 progress from less to more type erasure.
 // Binary-compatibility of std::functions is assumed up and down the hierarchy.

#include <functional>
template <class F> using Func = std::function<F>;

namespace hacc {

    template <class X, class M>
    struct GetSet2 {
         // In order to minimize copying of temporaries, get and set have to be
         // higher-order functions.  Using std::function imposes some overhead, but
         // not nearly as much as could potentially be imposed by having to copy
         // a large piece of data when we could update in place instead.

         // A getter takes an X and a function that will process the gotten value.
        typedef Func<void (const X&, const Func<void (const M&)>&)> Get;
         // A setter takes an X and a function that will update the settable value through a pointer.
        typedef Func<void (X&, const Func<void (M&)>&)> Set;
        Get get;
        Set set;
        GetSet2 (Get get, Set set) : get(get), set(set) { }
    };

    template <class X>
    struct GetSet1 {
        typedef Func<void (const X&, const Func<void (void*)>&)> Get;
        typedef Func<void (X&, const Func<void (void*)>&)> Set;
        const std::type_info* mtype;
        Get get;
        Set set;
        GetSet1 (const std::type_info& mtype, Get get, Set set) :
            mtype(&mtype), get(get), set(set)
        { }
        template <class M>
        GetSet1 (const GetSet2<X, M>& gs2) :
            mtype(&typeid(M)), get(*(Get*)&gs2.get), set(*(Set*)&gs2.set)
        { }
    };

    struct GetSet0 {
        typedef Func<void (void*, const Func<void (void*)>&)> Get;
        typedef Func<void (void*, const Func<void (void*)>&)> Set;
        const std::type_info* xtype;
        const std::type_info* mtype;
        Get get;
        Set set;
        GetSet0 () : xtype(null), mtype(null) { }
        GetSet0 (const std::type_info& xtype, const std::type_info& mtype, Get get, Set set) :
            xtype(&xtype), mtype(&mtype), get(get), set(set)
        { }
        template <class X>
        GetSet0 (const GetSet1<X>& gs1) :
            xtype(&typeid(X)), mtype(gs1.mtype), get(*(Get*)&gs1.get), set(*(Set*)&gs1.set)
        { }
        template <class X, class M>
        GetSet0 (const GetSet2<X, M>& gs2) :
            xtype(&typeid(X)), mtype(&typeid(M)), get(*(Get*)&gs2.get), set(*(Set*)&gs2.set)
        { }
    };

    template <class C, bool has_members = std::is_class<C>::value || std::is_union<C>::value> struct GetSet_Builders;
    template <class C> struct GetSet_Builders<C, false> {
        template <class M>
        static GetSet2<C, M> value_functions (const Func<M (const C&)>& g, const Func<void (C&, M)>& s) {
            return GetSet2<C, M>(
                [g, s](const C& x, const Func<void (const M&)>& c){ const M& tmp = g(x); c(tmp); },
                [g, s](C& x, const Func<void (M&)>& c){ M tmp; c(tmp); s(tmp); }
            );
        }
        template <class M>
        static GetSet2<C, M> ref_functions (const Func<const M& (const C&)>& g, const Func<void (C&, const M&)>& s) {
            return GetSet2<C, M>(
                [g, s](const C& x, const Func<void (const M&)>& c){ c(g(x)); },
                [g, s](C& x, const Func<void (M&)>& c){ M tmp; c(tmp); s(tmp); }
            );
        }
        template <class M>
        static GetSet2<C, M> ref_function (const Func<M& (C&)>& f) {
            return GetSet2<C, M>(
                [f](const C& x, const Func<void (const M&)>& c){ c(f(x)); },
                [f](C& x, const Func<void (M&)>& c){ c(f(x)); }
            );
        }
        template <class M>
        static GetSet2<C, M> assignable () {
            return GetSet2<C, M>(
                [](const C& x, const Func<void (const M&)>& c){ M tmp = x; c(tmp); },
                [](C& x, const Func<void (M&)>& c){ M tmp; c(tmp); x = tmp; }
            );
        }
    };
    template <class C> struct GetSet_Builders<C, true> : GetSet_Builders<C, false> {
        template <class M>
        static GetSet2<C, M> member (M C::* mp) {
            return GetSet2<C, M>(
                [mp](const C& x, const Func<void (const M&)>& c){ c(x.*mp); },
                [mp](C& x, const Func<void (M&)>& c){ c(x.*mp); }
            );
        }
        template <class M>
        static GetSet2<C, M> value_methods (M (C::* g )()const, void (C::* s )(M)) {
            return GetSet2<C, M>(
                [g, s](const C& x, const Func<void (const M&)>& c){ c((x.*g)()); },
                [g, s](C& x, const Func<void (M&)>& c){ M tmp; c(tmp); (x.*s)(tmp); }
            );
        }
        template <class M>
        static GetSet2<C, M> ref_methods (const M& (C::* g )()const, void (C::* s )(const M&)) {
            return GetSet2<C, M>(
                [g, s](const C& x, const Func<void (const M&)>& c){ c((x.*g)()); },
                [g, s](C& x, const Func<void (M&)>& c){ M tmp; c(tmp); (x.*s)(tmp); }
            );
        }
        template <class M>
        static GetSet2<C, M> ref_method (M& (C::* m )()) {
            return GetSet2<C, M>(
                [m](const C& x, const Func<void (const M&)>& c){ c((x.*m)()); },
                [m](C& x, const Func<void (M&)>& c){ c((x.*m)()); }
            );
        }
    };

}

#endif
