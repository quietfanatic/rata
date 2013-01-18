#ifndef HAVE_HACC_HACCABLE_GETSET_H
#define HAVE_HACC_HACCABLE_GETSET_H

 // This file provides three classes for encapsulating OO magic attributes.
 // GetSet2, GetSet1, and GetSet0 progress from less to more type erasure.
 // Binary-compatibility of std::functions is assumed up and down the hierarchy.

#include <functional>
template <class F> using Func = std::function<F>;

namespace hacc {

     // This file is starting to show its age already.
    template <class C> inline void instantiate_table ();

    struct Optional { };

    template <class M>
    struct Defaulter1 {
        Func<void (M&)> def;
        Defaulter1 (const Func<void (M&)>& def) : def(def) { }
        Defaulter1 (Null) { }
        Defaulter1 (Optional) : def([](M&){}) { }
    };

    struct Defaulter0 {
        Func<void (void*)> def;
        Defaulter0 (Null n = null) : def(n) { }
        template <class M>
        Defaulter0 (Defaulter1<M> def1) : def(*(Func<void (void*)>*)&def1.def) { }
    };

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
        bool copies_on_get;
        bool copies_on_set;
        Defaulter1<M> def;
        GetSet2 (Get get, Set set, bool cg = true, bool cs = true, Defaulter1<M> def = null) :
            get(get), set(set), copies_on_get(cg), copies_on_set(cs), def(def)
        { instantiate_table<M>(); }  // Be sure to instantiate M's HCB.
    };

    template <class X>
    struct GetSet1 {
        typedef Func<void (const X&, const Func<void (void*)>&)> Get;
        typedef Func<void (X&, const Func<void (void*)>&)> Set;
        const std::type_info* mtype;
        Get get;
        Set set;
        bool copies_on_get;
        bool copies_on_set;
        Defaulter0 def;
        GetSet1 (const std::type_info& mtype, Get get, Set set, bool gs = true, bool cs = true, Defaulter0 def = null) :
            mtype(&mtype), get(get), set(set), copies_on_get(gs), copies_on_set(cs), def(def)
        { }
        template <class M>
        GetSet1 (const GetSet2<X, M>& gs2) :
            mtype(&typeid(M)), get(*(Get*)&gs2.get), set(*(Set*)&gs2.set),
            copies_on_get(gs2.copies_on_get), copies_on_set(gs2.copies_on_set),
            def(gs2.def)
        { }
    };

    struct GetSet0 {
        typedef Func<void (void*, const Func<void (void*)>&)> Get;
        typedef Func<void (void*, const Func<void (void*)>&)> Set;
        const std::type_info* xtype;
        const std::type_info* mtype;
        Get get;
        Set set;
        bool copies_on_get;
        bool copies_on_set;
        Defaulter0 def;
        GetSet0 () : xtype(null), mtype(null) { }
        GetSet0 (const std::type_info& xtype, const std::type_info& mtype, Get get, Set set, bool gs = true, bool cs = true, Defaulter0 def = null) :
            xtype(&xtype), mtype(&mtype), get(get), set(set), copies_on_get(gs), copies_on_set(cs), def(def)
        { }
        template <class X>
        GetSet0 (const GetSet1<X>& gs1) :
            xtype(&typeid(X)), mtype(gs1.mtype), get(*(Get*)&gs1.get), set(*(Set*)&gs1.set),
            copies_on_get(gs1.copies_on_get), copies_on_set(gs1.copies_on_set), def(gs1.def)
        { }
        template <class X, class M>
        GetSet0 (const GetSet2<X, M>& gs2) :
            xtype(&typeid(X)), mtype(&typeid(M)), get(*(Get*)&gs2.get), set(*(Set*)&gs2.set),
            copies_on_get(gs2.copies_on_get), copies_on_set(gs2.copies_on_set), def(gs2.def)
        { }
        operator bool () { return mtype; }
    };

    template <class C, bool has_members = std::is_class<C>::value || std::is_union<C>::value> struct GetSet_Builders;
    template <class C> struct GetSet_Builders<C, false> {
        
        template <class M>
        static Defaulter1<M> def (const M& def) {
            return Defaulter1<M>([def](M& m){ m = def; });
        }
        static constexpr Null required = null;
        static constexpr Optional optional = Optional();

        template <class M>
        static GetSet2<C, M> value_functions (const Func<M (const C&)>& g, const Func<void (C&, M)>& s, const Defaulter0& def = required) {
            return GetSet2<C, M>(
                [g, s](const C& x, const Func<void (const M&)>& c){ const M& tmp = g(x); c(tmp); },
                [g, s](C& x, const Func<void (M&)>& c){ M tmp; c(tmp); s(x, tmp); },
                true,
                true,
                def
            );
        }
        template <class M>
        static GetSet2<C, M> ref_functions (const Func<const M& (const C&)>& g, const Func<void (C&, const M&)>& s, const Defaulter1<M>& def = required) {
            return GetSet2<C, M>(
                [g, s](const C& x, const Func<void (const M&)>& c){ c(g(x)); },
                [g, s](C& x, const Func<void (M&)>& c){ M tmp; c(tmp); s(x, tmp); },
                false,
                true,
                def
            );
        }
        template <class M>
        static GetSet2<C, M> ref_function (const Func<M& (C&)>& f, const Defaulter1<M>& def = required) {
            return GetSet2<C, M>(
                [f](const C& x, const Func<void (const M&)>& c){ c(f(x)); },
                [f](C& x, const Func<void (M&)>& c){ c(f(x)); },
                false,
                false,
                def
            );
        }
        template <class M>
        static GetSet2<C, M> assignable (const Defaulter1<M>& def = required) {
            return GetSet2<C, M>(
                [](const C& x, const Func<void (const M&)>& c){ M tmp = x; c(tmp); },
                [](C& x, const Func<void (M&)>& c){ M tmp; c(tmp); x = tmp; },
                true,
                true,
                def
            );
        }
        template <class M>
        static GetSet2<C, M> supertype (const Defaulter1<M>& def = required) {
            return GetSet2<C, M>(
                [](const C& x, const Func<void (const M&)>& c){ c(x); },
                [](C& x, const Func<void (M&)>& c){ c(x); },
                false,
                false,
                def
            );
        }
        template <class M, class... Args>
        static GetSet2<C, M> member (Args... args) { throw Error("hcb::member is not available for non-class types."); }
        template <class M, class... Args>
        static GetSet2<C, M> value_methods (Args... args) { throw Error("hcb::value_methods is not available for non-class types."); }
        template <class M, class... Args>
        static GetSet2<C, M> ref_methods (Args... args) { throw Error("hcb::ref_methods is not available for non-class types."); }
        template <class M, class... Args>
        static GetSet2<C, M> ref_method (Args... args) { throw Error("hcb::ref_method is not available for non-class types."); }
    };
    template <class C> struct GetSet_Builders<C, true> : GetSet_Builders<C, false> {
        static constexpr Null required = null;
        template <class M>
        static GetSet2<C, M> member (M C::* mp, const Defaulter1<M>& def = required) {
            return GetSet2<C, M>(
                [mp](const C& x, const Func<void (const M&)>& c){ c(x.*mp); },
                [mp](C& x, const Func<void (M&)>& c){ c(x.*mp); },
                false,
                false,
                def
            );
        }
        template <class M>
        static GetSet2<C, M> value_methods (M (C::* g )()const, void (C::* s )(M), const Defaulter1<M>& def = required) {
            return GetSet2<C, M>(
                [g, s](const C& x, const Func<void (const M&)>& c){ c((x.*g)()); },
                [g, s](C& x, const Func<void (M&)>& c){ M tmp; c(tmp); (x.*s)(tmp); },
                true,
                true,
                def
            );
        }
        template <class M>
        static GetSet2<C, M> ref_methods (const M& (C::* g )()const, void (C::* s )(const M&), const Defaulter1<M>& def = required) {
            return GetSet2<C, M>(
                [g, s](const C& x, const Func<void (const M&)>& c){ c((x.*g)()); },
                [g, s](C& x, const Func<void (M&)>& c){ M tmp; c(tmp); (x.*s)(tmp); },
                false,
                true,
                def
            );
        }
        template <class M>
        static GetSet2<C, M> ref_method (M& (C::* m )(), const Defaulter1<M>& def = required) {
            return GetSet2<C, M>(
                [m](const C& x, const Func<void (const M&)>& c){ c((x.*m)()); },
                [m](C& x, const Func<void (M&)>& c){ c((x.*m)()); },
                false,
                false,
                def
            );
        }
    };

     // For reifying subtype relationships.
    struct Caster0 {
        const std::type_info& subtype;
        void* (* up ) (void*);
        void* (* down ) (void*);
    };
    template <class Base, class Sub>
    struct Caster2 {
        operator Caster0 () { return Caster0{
            typeid(Sub),
            [](void* p){ return (void*)static_cast<Base*>((Sub*)p); },
            [](void* p){ return (void*)static_cast<Sub*>((Base*)p); }
        }; }
    };



}

#endif
