#ifndef HAVE_HACC_TYPES_INTERNAL_H
#define HAVE_HACC_TYPES_INTERNAL_H

#include <typeindex>
#include <unordered_map>
#include "../inc/types.h"
#include "../inc/dynamicism.h"

namespace hacc {

    struct GetSetData : DPtee {
        bool optional = false;
        bool readonly = false;
        bool narrow = false;
        Type t;
        Type ht;
        GetSetData (Type t, Type ht) : t(t), ht(ht) { }
        virtual String description () const = 0;
        virtual void* address (void*) const = 0;
        virtual void* ro_address (void*) const = 0;
        virtual void get (void*, void*) const = 0;
        virtual void set (void*, void*) const = 0;
    };

    struct GS_Ptr : GetSetData {
        GS_Ptr (Type t) : GetSetData(t, t) { }
        String description () const { return "<Converted from Pointer>"; }
        void* address (void* c) const { return c; }
        void* ro_address (void* c) const { return c; }
        void get (void* c, void* m) const { t.copy_assign(m, c); }
        void set (void* c, void* m) const { t.copy_assign(c, m); }
    };

    struct GS_Chain : GetSetData {
        GetSet0 l;
        GetSet0 r;
        GS_Chain (GetSet0 l, GetSet0 r) :
            GetSetData(l.host_type(), r.type()), l(l), r(r)
        { }
        String description () const { return "(" + l.description() + " => " + r.description() + ")"; }
        void* address (void* c) const {
            void* mid = r.address(c);
            return mid ? l.address(mid) : mid;
        }
        void* ro_address (void* c) const {
            void* mid = r.ro_address(c);
            return mid ? l.ro_address(mid) : mid;
        }
        void get (void* c, void* m) const {
            l.type().stalloc([&](void* mid){
                l.get(c, mid);
                r.get(mid, m);
            });
        }
        void set (void* c, void* m) const {
            l.type().stalloc([&](void* mid){
                l.get(c, mid);
                r.set(mid, m);
                l.set(c, mid);
            });
        }
    };

    std::vector<TypeData*>& types_to_init ();
    std::unordered_map<std::type_index, TypeData*>& types_by_cpptype ();
    std::unordered_map<String, TypeData*>& types_by_name ();

    struct TypeData {
         // General
        const std::type_info* cpptype = null;
        size_t size = -1;
        void (* construct )(void*) = null;
        void (* destruct )(void*) = null;
        void (* copy_assign )(void*, void*) = null;
        void (* stalloc )(const Func<void (void*)>&) = null;
        Func<String ()> name = null;
        void (* describe )() = null;
        bool (* eq )(void*, void*) = null;
        GetSet0 gs_id = null;
         // Hacc-specific
        GetSet0 keys = null;
        Func<Reference (void*, String)> attrs_f = null;
        std::vector<std::pair<String, GetSet0>> attr_list;
        GetSet0 length = null;
        Func<Reference (void*, size_t)> elems_f = null;
        std::vector<GetSet0> elem_list;
        std::vector<std::pair<String, Dynamic>> value_list;
        Func<Tree* (void*)> to_tree = null;
        Func<void (void*, Tree*)> prepare = null;
        Func<void (void*, Tree*)> fill = null;
        Func<void (void*, Tree*)> finish = null;
        GetSet0 delegate = null;
        TypeData* pointee_type = null;
        bool initialized = false;

        TypeData (
            const std::type_info& cpptype,
            size_t size,
            void (* construct )(void*),
            void (* destruct )(void*),
            void (* copy_assign )(void*, void*),
            void (* stalloc )(const Func<void (void*)>&)
        ) :
            cpptype(&cpptype), size(size),
            construct(construct), destruct(destruct),
            copy_assign(copy_assign),
            stalloc(stalloc),
            gs_id(new GS_Ptr(this))
        {
            types_to_init().push_back(this);
        }
    };

}

#endif
