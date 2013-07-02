#ifndef HAVE_HACC_GETSETS_H
#define HAVE_HACC_GETSETS_H

#include "dynamicism.h"

namespace hacc {

    struct GS_ID0 : GetSet0 {
        Type _type;
        GS_ID0 (Type t) : _type(t) { }
        Type type () { return _type; }
        Type host_type () { return _type; }
        String description () { return "Pointer"; }
        void* address (void* c) { return c; }
        void get (void* c, const Cont& f) { f(c); }
        void set (void* c, const Cont& f) { f(c); }
        void mod (void* c, const Cont& f) { f(c); }
    };

    template <class C>
    struct GS_ID : GetSet2<C, C> {
        String description () { return "self"; }
        void* address (void* c) { return c; }
        void get (void* c, const Cont& f) { f(c); }
        void set (void* c, const Cont& f) { f(c); }
        void mod (void* c, const Cont& f) { f(c); }
    };

    template <class C, class M>
    struct GS_Member : GetSet2<C, M> {
        M C::* mp;
        GS_Member (M C::* mp) : mp(mp) { }
        String description () { return "member"; }
        void* address (void* c) { return &(((C*)c)->*mp); }
        void get (void* c, const Cont& f) { f(&(((C*)c)->*mp)); }
        void set (void* c, const Cont& f) { f(&(((C*)c)->*mp)); }
        void mod (void* c, const Cont& f) { f(&(((C*)c)->*mp)); }
    };

    template <class C, class M>
    struct GS_ValueFuncs : GetSet2<C, M> {
        Func<M (const C&)> getter;
        Func<void (C&, M)> setter;
        GS_ValueFuncs (const Func<M (const C&)>& getter, const Func<void (C&, M)>& setter) :
            getter(getter), setter(setter)
        { }
        String description () { return "value_funcs"; }
        void* address (void* c) { return NULL; }
        void get (void* c, const Cont& f) {
            const M& tmp = getter(*(C*)c);
            f((void*)&tmp);
        }
        void set (void* c, const Cont& f) {
            M tmp;
            f(&tmp);
            setter(*(C*)c, tmp);
        }
        void mod (void* c, const Cont& f) {
            M tmp = getter(*(C*)c);
            f(&tmp);
            setter(*(C*)c, tmp);
        }
    };

    template <class C, class M>
    struct GS_RefFuncs : GetSet2<C, M> {
        Func<const M& (const C&)> getter;
        Func<void (C&, const M&)> setter;
        GS_RefFuncs (const Func<const M& (const C&)>& getter, const Func<void (C&, const M&)>& setter) :
            getter(getter), setter(setter)
        { }
        String description () { return "ref_funcs"; }
        void* address (void* c) { return NULL; }
        void get (void* c, const Cont& f) {
            f(&getter(*(C*)c));
        }
         // This one is not quite satisfactory, as it requires an extra copy.
        void set (void* c, const Cont& f) {
            M tmp;
            f(&tmp);
            setter(*(C*)c, tmp);
        }
        void mod (void* c, const Cont& f) {
            M tmp = getter(*(C*)c);
            f(&tmp);
            setter(*(C*)c, tmp);
        }
    };

    template <class C, class M>
    struct GS_RefFunc : GetSet2<C, M> {
        Func<M& (C&)> getter;
        GS_RefFunc (const Func<M& (C*)>& getter) : getter(getter) { }
        String description () { return "ref_func"; }
        void* address (void* c) { return &getter(*(C*)c); }
        void get (void* c, const Cont& f) { f(&getter(*(C*)c)); }
        void set (void* c, const Cont& f) { f(&getter(*(C*)c)); }
        void mod (void* c, const Cont& f) { f(&getter(*(C*)c)); }
    };

    template <class C, class M>
    struct GS_ValueMethods : GetSet2<C, M> {
        M (C::* getter) () const;
        void (C::* setter) (M);
        GS_ValueMethods (M (C::* getter) () const, void (C::* setter) (M)) :
            getter(getter), setter(setter)
        { }
        String description () { return "value_methods"; }
        void* address (void* c) { return NULL; }
        void get (void* c, const Cont& f) {
            const M& tmp = (((C*)c)->*getter)();
            f((void*)&tmp);
        }
        void set (void* c, const Cont& f) {
            M tmp;
            f(&tmp);
            (((C*)c)->*setter)(tmp);
        }
        void mod (void* c, const Cont& f) {
            M tmp = (((C*)c)->*getter)();
            f(&tmp);
            (((C*)c)->*setter)(tmp);
        }
    };

    template <class C, class M>
    struct GS_RefMethods : GetSet2<C, M> {
        const M& (C::* getter) () const;
        void (C::* setter) (const M&);
        GS_RefMethods (const M& (C::* getter) () const, void (C::* setter) (const M&)) :
            getter(getter), setter(setter)
        { }
        String description () { return "ref_methods"; }
        void* address (void* c) { return NULL; }
        void get (void* c, const Cont& f) {
            f(&(((C*)c)->*getter)());
        }
        void set (void* c, const Cont& f) {
            M tmp;
            f(&tmp);
            (((C*)c)->*setter)(tmp);
        }
        void mod (void* c, const Cont& f) {
            M tmp = (((C*)c)->*getter)();
            f(&tmp);
            (((C*)c)->*setter)(tmp);
        }
    };
    
    template <class C, class M>
    struct GS_RefMethod : GetSet2<C, M> {
        M& (C::* getter) ();
        GS_RefMethod (M& (C::* getter) ()) : getter(getter) { }
        String description () { return "ref_method"; }
        void* address (void* c) { return &(((C*)c)->*getter)(); }
        void get (void* c, const Cont& f) { f(&(((C*)c)->*getter)()); }
        void set (void* c, const Cont& f) { f(&(((C*)c)->*getter)()); }
        void mod (void* c, const Cont& f) { f(&(((C*)c)->*getter)()); }
    };

    template <class C, class M>
    struct GS_Base : GetSet2<C, M> {
        String decription () { return "base"; }
        void* address (void* c) { return static_cast<M*>((C*)c); }
        void get (void* c, const Cont& f) { f(static_cast<M*>((C*)c)); }
        void set (void* c, const Cont& f) { f(static_cast<M*>((C*)c)); }
        void mod (void* c, const Cont& f) { f(static_cast<M*>((C*)c)); }
    };

    template <class C, class M>
    struct GS_Assignable : GetSet2<C, M> {
        String description () { return "assignable"; }
        void* address (void* c) { return NULL; }
        void get (void* c, const Cont& f) {
            M tmp = *(C*)c;
            f(&tmp);
        }
        void set (void* c, const Cont& f) {
            M tmp;
            f(&tmp);
            *(C*)c = tmp;
        }
        void mod (void* c, const Cont& f) {
            M tmp = *(C*)c;
            f(&tmp);
            *(C*)c = tmp;
        }
    };

    template <class C, bool has_members = std::is_class<C>::value || std::is_union<C>::value>
    struct Inheritable_GetSets;
    template <class C>
    struct Inheritable_GetSets<C, false> {
        template <class M>
        static GetSet2<C, M>* value_funcs (const Func<M (const C&)>& g, const Func<void (C&, M)>& s) {
            return new GS_ValueFuncs<C, M>{g, s};
        }
        template <class M>
        static GetSet2<C, M>* ref_funcs (const Func<const M& (const C&)>& g, const Func<void (C&, const M&)>& s) {
            return new GS_RefFuncs<C, M>{g, s};
        }
        template <class M>
        static GetSet2<C, M>* ref_func (const Func<M& (C&)>& f) {
            return new GS_RefFunc<C, M>{f};
        }
        template <class M>
        static GetSet2<C, M>* base () {
            return new GS_Base<C, M>();
        }
        template <class M>
        static GetSet2<C, M>* assignable () {
            return new GS_Assignable<C, M>();
        }
        static constexpr void* member = null;
        static constexpr void* value_methods = null;
        static constexpr void* ref_methods = null;
        static constexpr void* ref_method = null;
    };
    template <class C>
    struct Inheritable_GetSets<C, true> : Inheritable_GetSets<C, false> {
        template <class M>
        static GetSet2<C, M>* member (M C::* mp) {
            return new GS_Member<C, M>{mp};
        }
        template <class M>
        static GetSet2<C, M>* value_methods (M (C::* g )()const, void (C::* s )(M)) {
            return new GS_ValueMethods<C, M>{g, s};
        }
        template <class M>
        static GetSet2<C, M>* ref_methods (const M& (C::* g )()const, void (C::* s )(const M&)) {
            return new GS_RefMethods<C, M>{g, s};
        }
        template <class M>
        static GetSet2<C, M>* ref_method (M& (C::* m )()) { return new GS_RefMethod<C, M>{m}; }
    };

}

#endif
