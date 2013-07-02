#ifndef HAVE_HACC_HACCABLE_H
#define HAVE_HACC_HACCABLE_H

#include <unordered_map>
#include "dynamicism.h"
#include "getsets.h"
#include "../../util/inc/annotations.h"

namespace hacc {

     // INTERNAL STUFF (type-erased versions of declaration api functions)
    void _name (Type, const Func<String ()>&);
    void _keys (Type, GetSet0*);
    void _attrs (Type, const Func<Reference (void*, String)>&);
    void _attr (Type, String, GetSet0*);
    void _length (Type, GetSet0*);
    void _elems (Type, const Func<Reference (void*, size_t)>&);
    void _elem (Type, GetSet0*);
    void _to_tree (Type, const Func<Tree* (void*)>&);
    void _delegate (Type, GetSet0*);
    void _prepare (Type, const Func<void (void*, Tree*)>&);
    void _fill (Type, const Func<void (void*, Tree*)>&);
    void _finish (Type, const Func<void (void*, Tree*)>&);
    void _is_raw_pointer (Type, Type);

     // DECLARATION API

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
        static void keys (GetSet2<C, std::vector<std::string>>* gs) {
            _keys(Type::CppType<C>(), gs);
        }
        static void attrs (const Func<Reference (C&, String)>& f) {
            _attrs(Type::CppType<C>(), reinterpret_cast<const Func<Reference (void*, String)>&>(f));
        }
        static void attr (String name, GetSet1<C>* gs) {
            _attr(Type::CppType<C>(), name, gs);
        }
        static void length (GetSet2<C, size_t>* gs) {
            _length(Type::CppType<C>(), gs);
        }
        static void elems (const Func<Reference (C&, size_t)>& f) {
            _elems(Type::CppType<C>(), reinterpret_cast<const Func<Reference (void*, size_t)>&>(f));
        }
        static void elem (GetSet1<C>* gs) {
            _elem(Type::CppType<C>(), gs);
        }
        static void delegate (GetSet1<C>* gs) {
            _delegate(Type::CppType<C>(), gs);
        }
        static void to_tree (const Func<Tree* (const C&)>& f) {
            _to_tree(Type::CppType<C>(), reinterpret_cast<const Func<Tree* (void*)>&>(f));
        }
        static void prepare (const Func<void (C&, Tree*)>& f) {
            _prepare(Type::CppType<C>(), reinterpret_cast<const Func<void (void*, Tree*)>&>(f));
        }
        static void fill (const Func<void (C&, Tree*)>& f) {
            _fill(Type::CppType<C>(), reinterpret_cast<const Func<void (void*, Tree*)>&>(f));
        }
        static void finish (const Func<void (C&, Tree*)>& f) {
            _finish(Type::CppType<C>(), reinterpret_cast<const Func<void (void*, Tree*)>&>(f));
        }
        static void is_raw_pointer (Type t) {
            _is_raw_pointer(Type::CppType<C>(), t);
        }
        static Type get_type () {
            static Type dt = _get_type(
                typeid(C), sizeof(C),
                [](void* p){ new (p) C; },
                [](void* p){ ((C*)p)->~C(); },
                [](void* to, void* from){ new (to) C (*(const C*)from); },
                TypeDecl<C>::describe
            );
            return dt;
        }
    };

}

#define HACC_APPEND(a, b) a##b
#define HACC_APPEND2(a, b) HACC_APPEND(a, b)
#define __ HACC_APPEND2(_anon_, __COUNTER__)
#define HCB_INSTANCE(type) static Type __ __attribute__ ((unused)) = TypeDecl<type>::get_type();
#define HCB_BEGIN(type) namespace hacc { template <> struct TypeDecl<type> : hacc::Haccability<type> { static void describe () {
#define HCB_END(type) } }; static Type __ __attribute__ ((unused)) = TypeDecl<type>::get_type(); }
#define HCB_PARAMS(...) __VA_ARGS__
#define HCB_COMMA ,
#define HCB_TEMPLATE_BEGIN(params, type) namespace hacc { template params struct TypeDecl<type> : hacc::Haccability<type> { \
    using hcb = hacc::Haccability<type>; \
    using hcb::name; \
    using hcb::keys; \
    using hcb::attrs; \
    using hcb::attr; \
    using hcb::length; \
    using hcb::elems; \
    using hcb::elem; \
    using hcb::delegate; \
    using hcb::to_tree; \
    using hcb::prepare; \
    using hcb::fill; \
    using hcb::finish; \
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
#define HCB_TEMPLATE_END(params, type) } }; }  // Reserved in case we need to do some magic static-var wrangling

#endif
