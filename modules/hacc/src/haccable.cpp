#include "../inc/haccable.h"
#include "types_internal.h"

namespace hacc {

    void _name (Type t, const Func<String ()>& name) {
        t.data->name = name;
    }
    void _keys (Type t, const GetSet0& gs) {
        t.data->keys = gs;
    }
    void _attrs (Type t, const Func<Reference (void*, String)>& f) {
        t.data->attrs_f = f;
    }
    void _attr (Type t, String name, const GetSet0& gs) {
        t.data->attr_list.emplace_back(name, gs);
    }
    void _length (Type t, const GetSet0& gs) {
        t.data->length = gs;
    }
    void _elems (Type t, const Func<Reference (void*, size_t)>& f) {
        t.data->elems_f = f;
    }
    void _elem (Type t, const GetSet0& gs) {
        t.data->elem_list.emplace_back(gs);
    }
    void _value (Type t, String name, Dynamic&& v, bool(* eq )(void*, void*)) {
        t.data->value_list.emplace_back(name, std::forward<Dynamic>(v));
        t.data->eq = eq;
    }
    void _to_tree (Type t, const Func<Tree* (void*)>& f) {
        t.data->to_tree = f;
    }
    void _prepare (Type t, const Func<void (void*, Tree*)>& f) {
        t.data->prepare = f;
    }
    void _fill (Type t, const Func<void (void*, Tree*)>& f) {
        t.data->fill = f;
    }
    void _finish (Type t, const Func<void (void*, Tree*)>& f) {
        t.data->finish = f;
    }
    void _delegate (Type t, const GetSet0& gs) {
        if (t == gs.type())
            throw X::Recursive_Delegate(t, gs);
        t.data->delegate = gs;
    }
    void _is_raw_pointer (Type t, Type pt) {
        t.data->pointee_type = pt.data;
    }

    struct Value_Funcs : GetSetData {
        UnknownF2 g;
        UnknownF2 s;
        Value_Funcs(Type t, Type ht, const UnknownF2& g, const UnknownF2& s) :
           GetSetData(t, ht), g(g), s(s)
        { }
        String description () const { return "value_funcs"; }
        void* address (void* c) const { return null; }
        void* ro_address (void* c) const { return null; }
        void get (void* c, void* m) const { g(c, m); }
        void set (void* c, void* m) const { s(c, m); }
        GetSetData* clone () const { return new Value_Funcs(*this); }
    };
    GetSet0 _value_funcs (Type t, Type ht, const UnknownF2& g, const UnknownF2& s) {
        return new Value_Funcs(t, ht, g, s);
    }

    struct Mixed_Funcs : GetSetData {
        UnknownF2 g;
        UnknownF2 s;
        Mixed_Funcs(Type t, Type ht, const UnknownF2& g, const UnknownF2& s) :
           GetSetData(t, ht), g(g), s(s)
        { }
        String description () const { return "mixed_funcs"; }
        void* address (void* c) const { return null; }
        void* ro_address (void* c) const { return null; }
        void get (void* c, void* m) const { g(c, m); }
        void set (void* c, void* m) const { s(c, m); }
        GetSetData* clone () const { return new Mixed_Funcs(*this); }
    };
    GetSet0 _mixed_funcs (Type t, Type ht, const UnknownF2& g, const UnknownF2& s) {
        return new Mixed_Funcs(t, ht, g, s);
    }

    struct Ref_Funcs : GetSetData {
        UnknownF1 g;
        UnknownF2 s;
        Ref_Funcs(Type t, Type ht, const UnknownF1& g, const UnknownF2& s) :
           GetSetData(t, ht), g(g), s(s)
        { }
        String description () const { return "ref_funcs"; }
        void* address (void* c) const { return null; }
        void* ro_address (void* c) const { return g(c); }
        void get (void* c, void* m) const { t.copy_assign(m, g(c)); }
        void set (void* c, void* m) const { s(c, m); }
        GetSetData* clone () const { return new Ref_Funcs(*this); }
    };
    GetSet0 _ref_funcs (Type t, Type ht, const UnknownF1& g, const UnknownF2& s) {
        return new Ref_Funcs(t, ht, g, s);
    }

    struct Ref_Func : GetSetData {
        UnknownF1 f;
        Ref_Func(Type t, Type ht, const UnknownF1& f) :
           GetSetData(t, ht), f(f)
        { }
        String description () const { return "ref_func"; }
        void* address (void* c) const { return f(c); }
        void* ro_address (void* c) const { return f(c); }
        void get (void* c, void* m) const { t.copy_assign(m, f(c)); }
        void set (void* c, void* m) const { t.copy_assign(f(c), m); }
        GetSetData* clone () const { return new Ref_Func(*this); }
    };
    GetSet0 _ref_func (Type t, Type ht, const UnknownF1& f) {
        return new Ref_Func(t, ht, f);
    }

    struct Base : GetSetData {
        void*(* f )(void*);
        Base(Type t, Type ht, void*(* f )(void*)) :
           GetSetData(t, ht), f(f)
        { }
        String description () const { return "base"; }
        void* address (void* c) const { return f(c); }
        void* ro_address (void* c) const { return f(c); }
        void get (void* c, void* m) const { t.copy_assign(m, f(c)); }
        void set (void* c, void* m) const { t.copy_assign(f(c), m); }
        GetSetData* clone () const { return new Base(*this); }
    };
    GetSet0 base (Type t, Type ht, void*(* f )(void*)) {
        return new Base(t, ht, f);
    }

    struct Assignable : GetSetData {
        void(* g )(void*,void*);
        void(* s )(void*,void*);
        Assignable(Type t, Type ht, void(* g )(void*,void*), void(* s )(void*,void*)) :
           GetSetData(t, ht), g(g), s(s)
        { }
        String description () const { return "assignable"; }
        void* address (void* c) const { return null; }
        void* ro_address (void* c) const { return null; }
        void get (void* c, void* m) const { g(c, m); }
        void set (void* c, void* m) const { s(c, m); }
        GetSetData* clone () const { return new Assignable(*this); }
    };
    GetSet0 _assignable (Type t, Type ht, void(* g )(void*,void*), void(* s )(void*,void*)) {
        return new Assignable(t, ht, g, s);
    }

    struct Member : GetSetData {
        UnknownMP mp;
        Member(Type t, Type ht, UnknownMP mp) :
           GetSetData(t, ht), mp(mp)
        { }
        String description () const { return "member"; }
        void* address (void* c) const { return &(((Unknown*)c)->*mp); }
        void* ro_address (void* c) const { return &(((Unknown*)c)->*mp); }
        void get (void* c, void* m) const { t.copy_assign(m, &(((Unknown*)c)->*mp)); }
        void set (void* c, void* m) const { t.copy_assign(&(((Unknown*)c)->*mp), m); }
        GetSetData* clone () const { return new Member(*this); }
    };
    GetSet0 _member (Type t, Type ht, UnknownMP mp) {
        return new Member(t, ht, mp);
    }

    struct Value_Methods : GetSetData {
        UnknownF2 g;
        UnknownF2 s;
        Value_Methods(Type t, Type ht, const UnknownF2& g, const UnknownF2& s) :
           GetSetData(t, ht), g(g), s(s)
        { }
        String description () const { return "value_methods"; }
        void* address (void* c) const { return null; }
        void* ro_address (void* c) const { return null; }
        void get (void* c, void* m) const { g(c, m); }
        void set (void* c, void* m) const { s(c, m); }
        GetSetData* clone () const { return new Value_Methods(*this); }
    };
    GetSet0 _value_methods (Type t, Type ht, const UnknownF2& g, const UnknownF2& s) {
        return new Value_Methods(t, ht, g, s);
    }

    struct Mixed_Methods : GetSetData {
        UnknownF2 g;
        UnknownSP s;
        Mixed_Methods(Type t, Type ht, const UnknownF2& g, UnknownSP s) :
           GetSetData(t, ht), g(g), s(s)
        { }
        String description () const { return "mixed_methods"; }
        void* address (void* c) const { return null; }
        void* ro_address (void* c) const { return null; }
        void get (void* c, void* m) const { g(c, m); }
        void set (void* c, void* m) const { (((Unknown*)c)->*s)(m); }
        GetSetData* clone () const { return new Mixed_Methods(*this); }
    };
    GetSet0 _mixed_methods (Type t, Type ht, const UnknownF2& g, UnknownSP s) {
        return new Mixed_Methods(t, ht, g, s);
    }

    struct Ref_Methods : GetSetData {
        UnknownGP g;
        UnknownSP s;
        Ref_Methods(Type t, Type ht, UnknownGP g, UnknownSP s) :
           GetSetData(t, ht), g(g), s(s)
        { }
        String description () const { return "ref_methods"; }
        void* address (void* c) const { return null; }
        void* ro_address (void* c) const { return (((Unknown*)c)->*g)(); }
        void get (void* c, void* m) const { t.copy_assign(m, (((Unknown*)c)->*g)()); }
        void set (void* c, void* m) const { (((Unknown*)c)->*s)(m); }
        GetSetData* clone () const { return new Ref_Methods(*this); }
    };
    GetSet0 _ref_methods (Type t, Type ht, UnknownGP g, UnknownSP s) {
        return new Ref_Methods(t, ht, g, s);
    }

    struct Ref_Method : GetSetData {
        UnknownGP f;
        Ref_Method(Type t, Type ht, UnknownGP f) :
           GetSetData(t, ht), f(f)
        { }
        String description () const { return "ref_method"; }
        void* address (void* c) const { return (((Unknown*)c)->*f)(); }
        void* ro_address (void* c) const { return (((Unknown*)c)->*f)(); }
        void get (void* c, void* m) const { t.copy_assign(m, (((Unknown*)c)->*f)()); }
        void set (void* c, void* m) const { t.copy_assign((((Unknown*)c)->*f)(), m); }
        GetSetData* clone () const { return new Ref_Method(*this); }
    };
    GetSet0 _ref_method (Type t, Type ht, UnknownGP f) {
        return new Ref_Method(t, ht, f);
    }

    namespace X {
        Recursive_Delegate::Recursive_Delegate (Type t, const GetSet0& gs) :
            Logic_Error(
                "Type " + t.name() + " delegates to itself through " + gs.description()
            ), type(t), gs(gs)
        { }
    }

}

