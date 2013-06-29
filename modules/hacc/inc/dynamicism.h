#ifndef HAVE_HACC_DYNAMICISM_H
#define HAVE_HACC_DYNAMICISM_H

#include <functional>
#include <typeinfo>
#include <typeindex>
#include <gc/gc_cpp.h>
#include "tree.h"

namespace hacc {

     // Utilitous declarations
    template <class F> using Func = std::function<F>;
    typedef Func<void (void*)> Cont;

     // The main type type
    struct TypeData;
    struct Type {
        TypeData* data;

        String name () const;
        const std::type_info& cpptype () const;
        size_t size () const;
        void construct (void*) const;
        void destruct (void*) const;
        void copy_construct (void*, void*) const;
         // If this type is a raw pointer, returns the type it points to
        Type pointee_type () const;

        Type (TypeData* p) : data(p) { }
        Type (const std::type_info&);
        Type (String);
        bool operator == (const Type& other) const {
             // TypeDatas are guaranteed to not be duplicated.
            return data == other.data;
        }
    };
     // For internal use, returns null instead of throwing exception
    TypeData* typedata_by_cpptype (const std::type_info&);

     // For typeless null pointers and such
    struct Unknown { };

     // GetSets represent a way of getting and setting a value that
     //  belongs to another value.  GetSet0, GetSet1, and GetSet2
     //  all represent the same type, but with different numbers
     //  of compile-time-known types.
    struct GetSet0 : gc {
         // All getsets must support these being set
        bool optional = false;
        bool readonly = false;
        virtual Type type () = 0;
        virtual Type host_type () = 0;
        virtual String description () { return "<unknown GetSet>"; }
        virtual void* address (void*) = 0;
        virtual void get (void*, const Cont&) = 0;
        virtual void set (void*, const Cont&) = 0;
        virtual void mod (void*, const Cont&) = 0;
    };
    template <class C>
    struct GetSet1 : GetSet0 {
        Type host_type () { return typeid(C); }
    };
    template <class C, class M>
    struct GetSet2 : GetSet1<C> {
        Type type () { return typeid(M); }
         // Named-Parameter-Idiom setters
        GetSet2<C, M>& optional () { optional = true; return *this; }
        GetSet2<C, M>& required () { optional = false; return *this; }
        GetSet2<C, M>& readonly () { readonly = true; return *this; }
    };

     // A Pointer implements a dynamically-typed pointer.
    struct Pointer {
        Type type;
        void* address;

        Pointer (Null n = null) : type(typeid(Unknown)), address(null) { }
        Pointer (Type type, void* p = null) : type(type), address(p) { }
        template <class C>
        Pointer (C* p) : type(typeid(C)), address(p) { }

        operator void* () const { return address; }
        operator bool () const { return address; }
         // These throw if the types don't exactly match
        void* address_of_type (Type) const;
        template <class C>
        operator C* () const { return address_of_type(typeid(C)); }
    };
    
     // A Reference implements a dynamically-typed object that
     //  might not be addressable but can still be got or set.
    struct Reference {
        void* c;
        GetSet0* gs;

        Reference (Null n = null) : c(null), gs(null) { }
        Reference (void* c, GetSet0* gs) : c(c), gs(gs) { }
        Reference (Type type, void* p);
        Reference (Pointer p) : Reference(p.type, p.address) { }

         // If the data is not addressable, this returns null.
        void* address () const { return gs->address(c); }
        Type type () const { return gs->type(); }
        Type host_type () const { return gs->host_type(); }
        void get (const Cont& f) const { gs->get(c, f); }
        void set (const Cont& f) const { gs->set(c, f); }
        void mod (const Cont& f) const { gs->mod(c, f); }

         // This will throw if the data is not addressable.
        operator Pointer () const;

         // Hacc-specific
        std::vector<String> keys ();
        void set_keys (const std::vector<String>&);
        size_t length ();
        void set_length (size_t);
         // These require that this reference be addressable.
        Reference attr (String);
        Reference elem (size_t);
         // These might require addressability
        Tree* to_tree ();
        void prepare (Tree*);
        void fill (Tree*);
        void finish (Tree*);
        void from_tree (Tree*);
         // These are primarily for use by files.cpp
        void foreach_address (const Func<void (Pointer, Path*)>&, Path*);
         // The callback will be given a Reference to a raw pointer.
        void foreach_pointer (const Func<void (Reference)>&);
    };

     // This is a dynamically typed object with value-semantics.
     // Or, if you prefer, a dynamically type auto_ptr
    struct Dynamic {
        Type type;
        void* addr;

        Pointer address () const { return Pointer(type, addr); }

        Dynamic (Null n = null) : type(typeid(Unknown)), addr(null) { }
        Dynamic (const Dynamic& o) :
            type(o.type),
            addr(malloc(type.size()))
        {
            type.copy_construct(addr, o.addr);
        }
        Dynamic (Dynamic&& o) :
            type(o.type),
            addr(o.addr)
        {
            o.addr = null;
        }
        Dynamic (Type type, void* addr) : type(type), addr(addr) { }

        template <class C, class... Args>
        static Dynamic New (Args&&... args) {
            void* p = malloc(sizeof(C));
            new (p) C (std::forward<Args>(args)...);
            return Dynamic(typeid(C), p);
        }

        void destroy () {
            if (addr) {
                type.destruct(addr);
                free(addr);
                addr = null;
            }
        }
        ~Dynamic () { destroy(); }

        Dynamic& operator = (const Dynamic& o) {
            destroy();
            type = o.type;
            addr = malloc(type.size());
            type.copy_construct(addr, o.addr);
            return *this;
        }
        Dynamic& operator = (Dynamic&& o) {
            destroy();
            type = o.type;
            addr = o.addr;
            o.addr = null;
            return *this;
        }
    };

    namespace X {
        struct Type_Mismatch : Logic_Error {
            Type expected;
            Type got;
            Type_Mismatch (Type expected, Type got);
        };
        struct Unaddressable : Logic_Error {
            Reference r;
            String goal;
            Unaddressable (Reference, String);
        };
        struct Missing_Attr : Logic_Error {
            Type type;
            String name;
            Missing_Attr (Type, String);
        };
        struct Missing_Elem : Logic_Error {
            Type type;
            size_t index;
            Missing_Elem (Type, size_t);
        };
        struct Too_Long : Logic_Error {
            Type type;
            size_t wanted;
            size_t maximum;
            Too_Long (Type, size_t, size_t);
        };
        struct No_Attr : Logic_Error {
            Type type;
            String name;
            No_Attr (Type, String);
        };
        struct Out_Of_Range : Logic_Error {
            Type type;
            size_t index;
            size_t length;
            Out_Of_Range (Type, size_t, size_t);
        };
        struct No_Attrs : Logic_Error {
            Type type;
            String name;
            No_Attrs (Type);
            No_Attrs (Type, String);
        };
        struct No_Elems : Logic_Error {
            Type type;
            size_t index = 0;
            No_Elems (Type);
            No_Elems (Type, size_t);
        };
        struct No_Type_For_CppType : Logic_Error {
            const std::type_info& cpptype;
            No_Type_For_CppType (const std::type_info&);
        };
        struct No_Type_For_Name : Logic_Error {
            String name;
            No_Type_For_Name (String);
        };
    }

}

 // This is so we can use Pointers as the key in an unordered_map
namespace std {
    template <>
    struct hash<hacc::Pointer> {
        typedef hacc::Pointer argument_type;
        typedef size_t result_type;
        result_type operator () (argument_type p) {
            return std::hash<type_index>()(p.type.cpptype())
                 ^ std::hash<void*>()(p.address);
        }
    };
}

#endif
