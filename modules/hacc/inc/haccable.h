#ifndef HAVE_HACC_HACCABLE_H
#define HAVE_HACC_HACCABLE_H

#include <unordered_map>
#include "dynamicism.h"

namespace hacc {

    using UnknownMP = Unknown Unknown::*;
    using UnknownGP = void* (Unknown::*) ();
    using UnknownSP = void (Unknown::*) (void*);
    using UnknownF1 = Func<void* (void*)>;
    using UnknownF2 = Func<void (void*,void*)>;

     // INTERNAL STUFF (type-erased versions of declaration api functions)
    void _name (Type, const Func<String ()>&);
    void _keys (Type, const GetSet0&);
    void _attrs (Type, const Func<Reference (void*, String)>&);
    void _attr (Type, String, const GetSet0&);
    void _length (Type, const GetSet0&);
    void _elems (Type, const Func<Reference (void*, size_t)>&);
    void _elem (Type, const GetSet0&);
    void _value (Type, String, Dynamic&&, bool(*)(void*, void*));
    void _array (Type);
    void _to_tree (Type, const Func<Tree (void*)>&);
    void _prepare (Type, const Func<void (void*, Tree)>&);
    void _fill (Type, const Func<void (void*, Tree)>&);
    void _finish (Type, const Func<void (void*)>&);
    void _delegate (Type, const GetSet0&);
    void _is_raw_pointer (Type, Type);
    GetSet0 _value_funcs (Type, Type, const UnknownF2&, const UnknownF2&);
    GetSet0 _mixed_funcs (Type, Type, const UnknownF2&, const UnknownF2&);
    GetSet0 _ref_funcs (Type, Type, const UnknownF1&, const UnknownF2&);
    GetSet0 _ref_func (Type, Type, const UnknownF1&);
    GetSet0 _base (Type, Type, void*(*)(void*));
    GetSet0 _assignable (Type, Type, void(*)(void*,void*), void(*)(void*,void*));
    GetSet0 _member (Type, Type, UnknownMP);
    GetSet0 _value_methods (Type, Type, const UnknownF2&, const UnknownF2&);
    GetSet0 _mixed_methods (Type, Type, const UnknownF2&, UnknownSP);
    GetSet0 _ref_methods (Type, Type, UnknownGP, UnknownSP);
    GetSet0 _ref_method (Type, Type, UnknownGP);

     // DECLARATION API
    template <class C, bool has_members = std::is_class<C>::value || std::is_union<C>::value>
    struct Inheritable_GetSets;
    template <class C>
    struct Inheritable_GetSets<C, false> {
        template <class M>
        static GetSet2<C, M> value_funcs (const Func<M (const C&)>& g, const Func<void (C&, M)>& s) {
            return static_cast<GetSet2<C, M>&&>(_value_funcs(
                Type::CppType<M>(), Type::CppType<C>(),
                [g](void* c, void* m){ *(M*)m = g(*(C*)c); },
                [s](void* c, void* m){ s(*(C*)c, *(M*)m); }
            ));
        }
        template <class M>
        static GetSet2<C, M> ref_funcs (const Func<const M& (const C&)>& g, const Func<void (C&, const M&)>& s) {
            printf("Called ref_funcs on %s (%s)\n", typeid(C).name(), typeid(M).name());
            return static_cast<GetSet2<C, M>&&>(_ref_funcs(
                Type::CppType<M>(), Type::CppType<C>(),
                reinterpret_cast<const UnknownF1&>(g),
                reinterpret_cast<const UnknownF2&>(s)
            ));
        }
        template <class M>
        static GetSet2<C, M> mixed_funcs (const Func<M (const C&)>& g, const Func<void (C&, const M&)>& s) {
            return static_cast<GetSet2<C, M>&&>(_mixed_funcs(
                Type::CppType<M>(), Type::CppType<C>(),
                [g](void* c, void* m){ *(M*)m = g(*(C*)c); },
                reinterpret_cast<const UnknownF2&>(s)
            ));
        }
        template <class M>
        static GetSet2<C, M> ref_func (const Func<M& (C&)>& f) {
            return static_cast<GetSet2<C, M>&&>(_ref_func(
                Type::CppType<M>(), Type::CppType<C>(),
                reinterpret_cast<const UnknownF1&>(f)
            ));
        }
        template <class M>
        static GetSet2<C, M> base () {
            return static_cast<GetSet2<C, M>&&>(_base(
                Type::CppType<M>(), Type::CppType<C>(),
                [](void* c){ return (void*)static_cast<M*>((C*)c); }
            ));
        }
        template <class M>
        static GetSet2<C, M> assignable () {
            return static_cast<GetSet2<C, M>&&>(_assignable(
                Type::CppType<M>(), Type::CppType<C>(),
                [](void* c, void* m){ *(M*)m = *(C*)c; },
                [](void* c, void* m){ *(C*)c = *(M*)m; }
            ));
        }
        static constexpr void* member = null;
        static constexpr void* value_methods = null;
        static constexpr void* ref_methods = null;
        static constexpr void* ref_method = null;
    };
    template <class C>
    struct Inheritable_GetSets<C, true> : Inheritable_GetSets<C, false> {
        template <class M>
        static GetSet2<C, M> member (M C::* mp) {
            return static_cast<GetSet2<C, M>&&>(_member(
                Type::CppType<M>(), Type::CppType<C>(),
                reinterpret_cast<UnknownMP&>(mp)
            ));
        }
        template <class M>
        static GetSet2<C, M> value_methods (M (C::* g )()const, void (C::* s )(M)) {
            return static_cast<GetSet2<C, M>&&>(_value_methods(
                Type::CppType<M>(), Type::CppType<C>(),
                [g](void* c, void* m){ *(M*)m = (((C*)c)->*g)(); },
                [s](void* c, void* m){ (((C*)c)->*s)(*(M*)m); }
            ));
        }
        template <class M>
        static GetSet2<C, M> ref_methods (const M& (C::* g )()const, void (C::* s )(const M&)) {
            return static_cast<GetSet2<C, M>&&>(_ref_methods(
                Type::CppType<M>(), Type::CppType<C>(),
                reinterpret_cast<UnknownGP&>(g),
                reinterpret_cast<UnknownSP&>(s)
            ));
        }
        template <class M>
        static GetSet2<C, M> mixed_methods (M (C::* g )()const, void (C::* s )(const M&)) {
            return static_cast<GetSet2<C, M>&&>(_mixed_methods(
                Type::CppType<M>(), Type::CppType<C>(),
                [g](void* c, void* m){ *(M*)m = (((C*)c)->*g)(); },
                reinterpret_cast<UnknownSP&>(s)
            ));
        }
        template <class M>
        static GetSet2<C, M> ref_method (M& (C::* m )()) {
            return static_cast<GetSet2<C, M>&&>(_ref_method(
                Type::CppType<M>(), Type::CppType<C>(),
                reinterpret_cast<UnknownGP&>(m)
            ));
        }
         // It's not a getset but it still depends on C being a class
        static void finish (void (C::* m )()) {
            _finish(Type::CppType<C>(), [m](C& x){ (x.*m)(); });
        }
    };

     // This is inherited by every custom-instantiated type.
    template <class C> struct Haccability : Inheritable_GetSets<C> {
        static void name (String s) {
            _name(Type::CppType<C>(), [s](){ return s; });
        }
        static void name (const char* s) {
            _name(Type::CppType<C>(), [s](){ return String(s); });
        }
        static void name (const Func<String ()>& f) {
            _name(Type::CppType<C>(), f);
        }
        static void keys (const GetSet2<C, std::vector<std::string>>& gs) {
            _keys(Type::CppType<C>(), gs);
        }
        static void attrs (const Func<Reference (C&, String)>& f) {
            _attrs(Type::CppType<C>(), reinterpret_cast<const Func<Reference (void*, String)>&>(f));
        }
        static void attr (String name, const GetSet1<C>& gs) {
            _attr(Type::CppType<C>(), name, gs);
        }
        static void length (const GetSet2<C, size_t>& gs) {
            _length(Type::CppType<C>(), gs);
        }
        static void elems (const Func<Reference (C&, size_t)>& f) {
            _elems(Type::CppType<C>(), reinterpret_cast<const Func<Reference (void*, size_t)>&>(f));
        }
        static void elem (const GetSet1<C>& gs) {
            _elem(Type::CppType<C>(), gs);
        }
        static void value (String s, C&& v) {
            _value(Type::CppType<C>(), s, Dynamic(std::forward<C>(v)), [](void* l, void* r){ return *(C*)l == *(C*)r; });
        }
        static void array () {
            _array(Type::CppType<C>());
        }
        static void to_tree (const Func<Tree (const C&)>& f) {
            _to_tree(Type::CppType<C>(), reinterpret_cast<const Func<Tree (void*)>&>(f));
        }
        static void prepare (const Func<void (C&, Tree)>& f) {
            _prepare(Type::CppType<C>(), reinterpret_cast<const Func<void (void*, Tree)>&>(f));
        }
        static void fill (const Func<void (C&, Tree)>& f) {
            _fill(Type::CppType<C>(), reinterpret_cast<const Func<void (void*, Tree)>&>(f));
        }
        static void finish (const Func<void (C&)>& f) {
            _finish(Type::CppType<C>(), reinterpret_cast<const Func<void (void*)>&>(f));
        }
        static void delegate (const GetSet1<C>& gs) {
            _delegate(Type::CppType<C>(), gs);
        }
        static void is_raw_pointer (Type t) {
            _is_raw_pointer(Type::CppType<C>(), t);
        }

        static Type get_type () {
            static Type dt = _get_type(
                typeid(C), sizeof(C),
                Constructibility<C>::construct,
                Destructibility<C>::destruct,
                Assignability<C>::assign,
                Constructibility<C>::stalloc,
                Hacc_TypeDecl<C>::describe
            );
            return dt;
        }
    };

    namespace X {
        struct Recursive_Delegate : Logic_Error {
            Type type;
            GetSet0 gs;
            Recursive_Delegate (Type, const GetSet0&);
        };
    };

}

#define HACC_APPEND(a, b) a##b
#define HACC_APPEND2(a, b) HACC_APPEND(a, b)
#define HACC_ANON HACC_APPEND2(_anon_, __COUNTER__)
#define HCB_INSTANCE(type) static hacc::Type HACC_ANON __attribute__ ((unused)) = Hacc_TypeDecl<type>::get_type();
#define HCB_BEGIN(type) template <> struct Hacc_TypeDecl<type> : hacc::Haccability<type> { static void describe () {
#define HCB_END(type) } }; static hacc::Type HACC_ANON __attribute__ ((unused)) = Hacc_TypeDecl<type>::get_type();
#define HCB_PARAMS(...) __VA_ARGS__
#define HCB_COMMA ,
#define HCB_TEMPLATE_BEGIN(params, type) template params struct Hacc_TypeDecl<type> : hacc::Haccability<type> { \
    using hcb = hacc::Haccability<type>; \
    using hcb::name; \
    using hcb::keys; \
    using hcb::attrs; \
    using hcb::attr; \
    using hcb::length; \
    using hcb::elems; \
    using hcb::elem; \
    using hcb::array; \
    using hcb::to_tree; \
    using hcb::prepare; \
    using hcb::fill; \
    using hcb::finish; \
    using hcb::delegate; \
    using hcb::is_raw_pointer; \
    using hcb::member; \
    using hcb::value_funcs; \
    using hcb::ref_funcs; \
    using hcb::ref_func; \
    using hcb::value_methods; \
    using hcb::ref_methods; \
    using hcb::ref_method; \
    using hcb::base; \
    using hcb::assignable; \
    static void describe () {
#define HCB_TEMPLATE_END(params, type) } };  // Reserved in case we need to do some magic static-var wrangling

#endif
