#ifndef HAVE_HACC_DYNAMICISM_H
#define HAVE_HACC_DYNAMICISM_H

#include <functional>
#include <gc/gc_cpp.h>

namespace hacc {

    template <class F> using Func = std::function<F>;
    typedef Type Type;
    typedef Func<void (void*)> Cont;

    struct GetSet0 : gc {
        const Type* type;
        bool optional = false;
        bool readonly = false;
        virtual void* address (void*) = 0;
        virtual void get (void*, const Cont&) = 0;
        virtual void set (void*, const Cont&) = 0;
        virtual void mod (void*, const Cont&) = 0;
        GetSet0 (const Type& type) : type(&type) { }
    };
    template <class C>
    struct GetSet1 : GetSet0 {
        GetSet1 (const Type& type) :
            GetSet0{type}
        { }
    };
    template <class C, class M>
    struct GetSet2<C, M> : GetSet1<C> {
        GetSet2 () : GetSet1{typeid(M)} { }
        GetSet2<C, M>& optional () { optional = true; return *this; }
        GetSet2<C, M>& required () { optional = false; return *this; }
        GetSet2<C, M>& readonly () { readonly = true; return *this; }
    };

    template <class C, class M>
    struct GS_Member : GetSet2<C, M> {
        C::*M mp;
        void* address (void* c) { return &((C*)c)->*mp; }
        void get (void* c, const Cont& f) { f(&((C*)c)->*mp); }
        void set (void* c, const Cont& f) { f(&((C*)c)->*mp); }
        void mod (void* c, const Cont& f) { f(&((C*)c)->*mp); }
    };

    template <class C, class M>
    struct GS_ValueFuncs : GetSet2<C, M> {
        std::function<M (const C&)> getter;
        std::function<void (C&, M)> setter;
        void* address (void* c) { return NULL; }
        void get (void* c, const Cont& f) {
            const M& tmp = getter(*(C*)c);
            f(&tmp);
        }
        void set (void* c, const Cont& f) {
            M tmp;
            f(&tmp);
            setter(*(C*)c, tmp);
        }
        void mod (void* c, const Cont& f) {
            const M& tmp = getter(*(C*)c);
            f(&tmp);
            setter(*(C*)c, tmp);
        }
    };

    template <class C, class M>
    struct GS_RefFuncs : GetSet2<C, M> {
        std::function<const M& (const C&)> getter;
        std::function<void (C&, const M&)> setter;
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
        std::function<M& (C&)> getter;
        void* address (void* c) { return &getter(*(C*)c); }
        void get (void* c, const Cont& f) { f(&getter(*(C*)c)); }
        void set (void* c, const Cont& f) { f(&getter(*(C*)c)); }
        void mod (void* c, const Cont& f) { f(&getter(*(C*)c)); }
    };

    template <class C, class M>
    struct GS_ValueMethods : GetSet2<C, M> {
        M (C::* getter) () const;
        void (C::* setter) (M);
        void* address (void* c) { return NULL; }
        void get (void* c, const Cont& f) {
            const M& tmp = (((C*)c)->*getter)();
            f(&tmp);
        }
        void set (void* c, const Cont& f) {
            M tmp;
            f(&tmp);
            (((C*)c)->*setter)(tmp);
        }
        void mod (void* c, const Cont& f) {
            M& tmp = (((C*)c)->*getter)();
            f(&tmp);
            (((C*)c)->*setter)(tmp);
        }
    };

    template <class C, class M>
    struct GS_RefMethods : GetSet2<C, M> {
        const M& (C::* getter) () const;
        void (C::* setter) (const M&);
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
        void* address (void* c) { return &(((C*)c)->*getter)(); }
        void get (void* c, const Cont& f) { f(&(((C*)c)->*getter)()); }
        void set (void* c, const Cont& f) { f(&(((C*)c)->*getter)()); }
        void mod (void* c, const Cont& f) { f(&(((C*)c)->*getter)()); }
    };

    template <class C, class M>
    struct GS_Base : GetSet2<C, M> {
        void* address (void* c) { return static_cast<M*>((C*)c); }
        void get (void* c, const Cont& f) { f(static_cast<M*>((C*)c); }
        void set (void* c, const Cont& f) { f(static_cast<M*>((C*)c); }
        void mod (void* c, const Cont& f) { f(static_cast<M*>((C*)c); }
    };

    template <class C, class M>
    struct GS_Assignable : GetSet2<C, M> {
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

    template <class C>
    struct GS_ID : GetSet2<C, C> {
        void* address (void* c) { return c; }
        void get (void* c, const Cont& f) { f(c); }
        void set (void* c, const Cont& f) { f(c); }
        void mod (void* c, const Cont& f) { f(c); }
    };

    struct Reference {
        void* c;
        const GetSet0* gs;
        Reference () : c(NULL), gs(NULL) { }
        Reference (void* c, const GetSet0* gs) : c(c), gs(gs) { }
        Reference (const Type& type_, void* c) :
            c(c), gs(new GS_ID<void>)
        {
            gs->type = &type_;
        }
        template <class C>
        Reference (C* c) : c(c), gs(new GS_ID<C>) { type = &typeid(C); }

        const Type& type () { return gs->type; }
        void get (const Cont& f) { gs->get(c, f); }
        void set (const Cont& f) { gs->set(c, f); }
        void mod (const Cont& f) { gs->mod(c, f); }
        void* address () { return gs->address(c); }

        template <class C> operator C* () {
            if (*gs->type == typeid(C)) {
                return (C*)c;
            }
            else throw hacc::Error("A type mismatch occurred when coercing from a Reference.  Sorry, better error message is NYI.");
        }
        operator bool () { return c && gs; }
    };

    struct Pointer {
        void* p;
        const Type* type;
        Pointer (const Type& type = *(const Type*)NULL, void* p = NULL) : p(p), type(&type) { }
        template <class C> Pointer (C* p) : p(p), type(&typeid(C)) { }
        Pointer (Reference r) : p(r.address()), type(r.type()) {
            if (!r.addressable)
                throw Error("Tried to convert unaddressable reference of type " + type_name(p.type()) + " into Pointer")
        }
        operator void* () const { return p; }
        operator bool () const { return p; }
        template <class C> operator C* () const {
            if (typeid(C) == *type) return (C*)p;
            else throw Error("Type mismatch: expected " + type_name<C>() + " but got " + type_name(*type));
        }
        operator Reference () const { return Reference{p, *type}; }
        operator std::pair<std::type_index, void*> () const {
            return std::pair<std::type_index, void*>(*type, p);
        }
    };

    struct Dynamic {
        void* p;
        HaccTable* ht;
        Dynamic (HaccTable* ht = NULL, void* p = NULL) : p(p), ht(ht) { }
        Dynamic (HaccTable* ht = NULL, void* p = NULL) : p(p), ht(ht) { }
        template <class C> explicit Dynamic (C* p) : p(p), ht(hacctable_by_type(typeid(C))) {
            if (!ht) {
                delete p;
                throw Error("Cannot construct a Dynamic out of non-haccable type " + type_name<C>());
            }
        }
        explicit Dynamic (Pointer p) : p(p.p), ht(hacctable_require_type(p.type)) { }
        ~Dynamic () {
            if (p && ht) {
                ht->destruct(p);
                delete (char*)p;  // This is kinda icky
            }
        }
        Pointer pointer () const { return Pointer(p, ht ? ht->type : NULL); }

        Dynamic (Dynamic&& o) : {
            if (p && ht) {
                ht->destruct(p);
                delete (char*)p;
            }
            p = o.p;
            o.p = NULL;
            ht = o.ht;
        }
    };

}

#endif
