#ifndef HAVE_HACC_TYPES_H
#define HAVE_HACC_TYPES_H

#include <unordered_map>
#include "dynamicism.h"
#include "getsets.h"
#include "../../util/inc/annotations.h"

namespace hacc {

     // INTERNAL STUFF (type-erased versions of declaration api functions)
    Type _new_type (const std::type_info&, size_t, void(*)(void*), void(*)(void*), void(*)(void*,void*));
    void _name (Type, String);
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
            _name(get_type(), s);
        }
        static void keys (GetSet2<C, std::vector<std::string>>* gs) {
            _keys(get_type(), gs);
        }
        static void attrs (const Func<Reference (C&, String)>& f) {
            _attrs(get_type(), reinterpret_cast<const Func<Reference (void*, String)>&>(f));
        }
        static void attr (String name, GetSet1<C>* gs) {
            _attr(get_type(), name, gs);
        }
        static void length (GetSet2<C, size_t>* gs) {
            _length(get_type(), gs);
        }
        static void elems (const Func<Reference (C&, size_t)>& f) {
            _elems(get_type(), reinterpret_cast<const Func<Reference (void*, size_t)>&>(f));
        }
        static void elem (GetSet1<C>* gs) {
            _elem(get_type(), gs);
        }
        static void delegate (GetSet1<C>* gs) {
            _delegate(get_type(), gs);
        }
        static void to_tree (const Func<Tree* (const C&)>& f) {
            _to_tree(get_type(), reinterpret_cast<const Func<Tree* (void*)>&>(f));
        }
        static void prepare (const Func<void (C&, Tree*)>& f) {
            _prepare(get_type(), reinterpret_cast<const Func<void (void*, Tree*)>&>(f));
        }
        static void fill (const Func<void (C&, Tree*)>& f) {
            _fill(get_type(), reinterpret_cast<const Func<void (void*, Tree*)>&>(f));
        }
        static void finish (const Func<void (C&, Tree*)>& f) {
            _finish(get_type(), reinterpret_cast<const Func<void (void*, Tree*)>&>(f));
        }
        static void is_raw_pointer (Type t) {
            _pointer(get_type(), t);
        }

        static Type get_type () {
            static Type type = typedata_by_cpptype(typeid(C));
            if (!type.data) {
                type = _new_type(
                    typeid(C), sizeof(C),
                    [](void* p){ new (p) C; },
                    [](void* p){ ((C*)p)->~C(); },
                    [](void* l, void* r){ new (l) C (*(C*)r); }
                );
                Haccable<C>::describe();
            }
            return type;
        }
    };

}

#define HACC_APPEND(a, b) a##b
#define HACC_APPEND2(a, b) HACC_APPEND(a, b)
#define __ HACC_APPEND2(_anon_, __COUNTER__)
#define HCB_INSTANCE(type) static Type __ __attribute__ ((unused)) = Haccable<type>::get_type();
#define HCB_BEGIN(type) namespace hacc { template <> struct Haccable<type> : hacc::Haccability<type> { static void describe () {
#define HCB_END(type) } }; static Type __ __attribute__ ((unused)) = Haccable<type>::get_type(); }
#define HCB_PARAMS(...) __VA_ARGS__
#define HCB_TEMPLATE_BEGIN(params, type) namespace hacc { template params struct Haccable<type> : hacc::Haccability<type> { \
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
