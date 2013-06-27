#ifndef HAVE_HACC_HACCABLE_H
#define HAVE_HACC_HACCABLE_H

#include <unordered_map>
#include "hacc.h"
#include "dynamicism.h"
#include "../../util/inc/annotations.h"

namespace hacc {

     // USAGE API
    String type_name (const Type&);
    template <class C>
    String type_name () { return type_name(typeid(C)); }

    std::vector<String> keys (Reference);
    Reference attr (Reference, String);
    size_t length (Reference);
    Reference elem (Reference, size_t);

     // Probably best to keep these to internal usage.
    Hacc* to_hacc (Reference);
    void update_from_hacc (Reference, Hacc*);

     // INTERNAL STUFF (type-erased versions of declaration api functions)
    struct HaccTable;
    HaccTable* hacctable_by_type (const Type&);
    HaccTable* hacctable_require_type (const Type&);
    HaccTable* hacctable_require_type_name (std::string);
    HaccTable* new_hacctable (const Type&, size_t, void(*)(void*), void(*)(void*));
    void _name (HaccTable*, const Func<String ()>&);
    void _keys (HaccTable*, GetSet0*);
    void _attrs (HaccTable*, const Func<Reference (void*, String)>&);
    void _attr (HaccTable*, String, GetSet0*);
    void _length (HaccTable*, GetSet0*);
    void _elems (HaccTable*, const Func<Reference (void*, size_t)>&);
    void _elem (HaccTable*, GetSet0*);
    void _to_hacc (HaccTable*, const Func<Hacc* (void*)>&);
    void _from_hacc (HaccTable*, const Func<void (void*, Hacc*)>&);
    void _delegate (HaccTable*, GetSet0*);
    void _prepare (HaccTable*, const Func<void (void*, Hacc*)>&);
    void _finish (HaccTable*, const Func<void (void*, Hacc*)>&);
    void _pointer (HaccTable*, GetSet0*);

     // DECLARATION API

     // This is specialized to make a type haccable.
    template <class C> struct Haccable {
        static HaccTable* get_table () {
            static HaccTable* table = hacctable_require_type(typeid(C));
            return table;
        }
    };
     // This is inherited by every custom-instantiated type.
    template <class C> struct Haccability {
        void name (const Func<String ()>& f) { _name(get_table(), f); }
        void name (String s) { _name(get_table(), [](){ return s; }); }
        void keys (GetSet2<C, std::vector<std::string>>* gs) {
            _keys(get_table(), gs);
        }
        void attrs (const Func<Reference (C&, String)>) {
            _attr(get_table(), reinterpret_cast<const Func<Reference (void*, String)>&>(f));
        }
        void attr (String name, GetSet1<C>* gs) { _attr(get_table(), name, gs); }
        void length (GetSet2<C, size_t>* gs) {
            _length(get_table(), gs);
        }
        void elems (const Func<Reference (C&, size_t)>& f) {
            _elem(get_table(), reinterpret_cast<const Func<Reference (void*, String)>&>(F));
        }
        void elem (GetSet1<C>* gs) { _elem(get_table(), gs); }
        void to_hacc (const Func<Hacc* (const C&)>& f) {
            _to_hacc(get_table(), reinterpret_cast<const Func<Hacc* (void*)>&>(f));
        }
        void from_hacc (const Func<void (C&, Hacc*)>& f) {
            _from_hacc(get_table(), reinterpret_cast<const Func<void (void*, Hacc*)>&>(f));
        }
        void delegate (GetSet1<C>* gs) { _delegate(get_table(), gs); }
        void prepare (const Func<void (C&, Hacc*)>& f) {
            _prepare(get_table(), reinterpret_cast<const Func<void (void*, Hacc*)>&>(f));
        }
        void finish (const Func<void (C& Hacc*)>& f) {
            _finish(get_table(), reinterpret_cast<const Func<void (void*, Hacc*)>&>(f));
        }
        void pointer (GetSet2<C, Pointer>* gs) { _pointer(get_table(), gs); }
        template <class M>
        GetSet2<C, M> member (C::*M mp) { return new GS_Member<C, M>{mp}; }
        template <class M>
        GetSet2<C, M> value_funcs (const Func<M (const C&)>& g, const Func<void (C&, M)>& s) {
            return new GS_ValueFuncs<C, M>{g, s};
        }
        template <class M>
        GetSet2<C, M> ref_funcs (const Func<const M& (const C&)>& g, const Func<void (C&, const M&)>& s) {
            return new GS_RefFuncs<C, M>{g, s};
        }
        template <class M>
        GetSet2<C, M> ref_func (const Func<M& (C&)>& f) { return new GS_RefFunc<C, M>{f}; }
        template <class M>
        GetSet2<C, M> value_methods (M (C::* g )()const, void (C::* s )(M)) {
            return new GS_ValueMethods<C, M>{g, s};
        }
        template <class M>
        GetSet2<C, M> ref_methods (const M& (C::* g )()const, void (C::* s )(const M&)) {
            return new GS_RefMethods<C, M>{g, s};
        }
        template <class M>
        GetSet2<C, M> ref_method (M& (C::* m )()) { return new GS_RefMethod<C, M>{m}; }
        template <class M>
        GetSet2<C, M> base () { return new GS_Base<C, M>(); }
        template <class M>
        GetSet2<C, M> assignable () { return new GS_Assignable<C, M>(); }

        static HaccTable* get_table () {
            static HaccTable* table = hacctable_by_type(typeid(C));
            if (!table) {
                table = new_hacctable(
                    typeid(C), sizeof(C),
                    [](void* p){ new (p) C; },
                    [](void* p){ ((C*)p)->~C(); }
                );
                Haccable<C>::describe();
            }
            return table;
        }
    };

     // PATHS AND ADDRESSES

     // path_to_address does not require any scans.
     // If a root is provided, the path loookup will start
     //  from there instead of the file indicated by the path.
     // Throws if the path doesn't resolve to a location
    Reference path_to_reference (Path*, Pointer root = null);

     // address_to_path may require all or some file-objects to be scanned.
     // If a root is provided, only paths starting at that root will be
     //  considered.  This can save time since it won't have to scan every
     //  single file-object.  Returns null if the address isn't found.
    Path* address_to_path (Pointer, Pointer root = null);

     // Free any memory associated with the address_to_path operation
     // You should do this as soon as possible after you're done using
     //  address_to_path, because there are no checks to make sure the scan
     //  data isn't stale.
    void clear_scans ();

     // Performs an operation for each pointer found in the given root, or in
     //  every file-object if root is null.  The callback will be always be
     //  provided with a Reference to Pointer.
    void foreach_pointer (const Func<void (Reference)>&, Pointer root = null);

}

#define HACC_APPEND(a, b) a##b
#define HACC_APPEND_COUNTER(a) HACC_APPEND(a, __COUNTER__)
#define __ HACC_APPEND_COUNTER(_anon_)
#define HCB_INSTANCE(type) static bool __ __attribute__ ((unused)) = Haccable<type>::get_table();
#define HCB_BEGIN(type) namespace hacc { template <> struct Haccable<type> : hacc::Haccability<type> { static void describe () {
#define HCB_END(type) } }; static bool __ __attribute__ ((unused)) = Haccable<type>::get_table(); }
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
    using hcb::to_hacc; \
    using hcb::from_hacc; \
    using hcb::delegate; \
    using hcb::prepare; \
    using hcb::finish; \
    using hcb::pointer; \
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
