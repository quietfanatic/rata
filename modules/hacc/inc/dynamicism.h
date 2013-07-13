#ifndef HAVE_HACC_DYNAMICISM_H
#define HAVE_HACC_DYNAMICISM_H

#include "tree.h"
#include "types.h"

namespace hacc {

     // GetSets represent a way of getting and setting a value that
     //  belongs to another value.  GetSet0, GetSet1, and GetSet2
     //  all represent the same type, but with different numbers
     //  of compile-time-known types.
    struct GetSetData;
    struct GetSet0 : DPtr<GetSetData> {
        GetSet0 (Null n = null) : DPtr(n) { }
        explicit GetSet0 (GetSetData* p) : DPtr(p) { }
        Type type () const;  // The type of the data
        Type host_type () const;  // The type of the reference the data came from
        String description () const;  // Says what kind of getset this is (value_funcs, etc.)
        void* address (void*) const;  // This returns null if the data is not addressable
        void* ro_address (void*) const;
        void get (void*, void*) const;  // Performs a copy
        void set (void*, void*) const;  // ''
         // Named-Parameter-Idiom setters
        GetSet0& optional ();
        GetSet0& required ();
        GetSet0& readonly ();
        GetSet0& narrow ();  // Do prepare and finish steps at fill time
        GetSet0& prepare ();  // Do fill and finish steps at prepare time
    };
    template <class C>
    struct GetSet1 : GetSet0 { };

    template <class C, class M>
    struct GetSet2 : GetSet1<C> {
        GetSet2<C, M>& optional () { return static_cast<GetSet2<C, M>&>(this->GetSet0::optional()); }
        GetSet2<C, M>& required () { return static_cast<GetSet2<C, M>&>(this->GetSet0::required()); }
        GetSet2<C, M>& readonly () { return static_cast<GetSet2<C, M>&>(this->GetSet0::readonly()); }
        GetSet2<C, M>& narrow () { return static_cast<GetSet2<C, M>&>(this->GetSet0::narrow()); }
        GetSet2<C, M>& prepare () { return static_cast<GetSet2<C, M>&>(this->GetSet0::prepare()); }
    };

     // A Pointer implements a dynamically-typed pointer.
    struct Pointer {
        Type type;
        void* address;

        Pointer (Null n = null) : type(Type::CppType<Unknown>()), address(null) { }
        Pointer (Type type, void* p = null) : type(type), address(p) { }
        template <class C>
        Pointer (C* p) : type(Type::CppType<C>()), address(p) { }

        operator void* () const { return address; }
        operator bool () const { return address; }
         // These throw if the types don't exactly match
        void* address_of_type (Type) const;
        template <class C>
        operator C* () const { return (C*)address_of_type(Type::CppType<C>()); }
    };
    static bool operator == (const Pointer& a, const Pointer& b) {
        return a.type == b.type && a.address == b.address;
    }
    
     // A Reference implements a dynamically-typed object that
     //  might not be addressable but can still be got or set.
    struct Reference {
        void* c;
        GetSet0 gs;

        Reference (Null n = null) : c(null), gs(null) { }
        Reference (void* c, const GetSet0& gs) : c(c), gs(gs) { }
        Reference (Type type, void* p);
        Reference (Pointer p) : Reference(p.type, p.address) { }

         // If the data is not addressable, this returns null.
        void* address () const { return gs.address(c); }
        void* ro_address () const { return gs.ro_address(c); }
        Type type () const { return gs.type(); }
        Type host_type () const { return gs.host_type(); }
        void get (void* m) const { gs.get(c, m); }
        void set (void* m) const { gs.set(c, m); }
         // Use ro_address or get, whichever is appropriate
        void read (const Func<void (void*)>& f) const;
         // Use address or set
        void write (const Func<void (void*)>& f) const;
         // Use address or get and set
        void mod (const Func<void (void*)>& f) const;

         // This will throw if the data is not addressable.
        operator Pointer () const;
        template <class C>
        operator C* () const { return Pointer(*this); }

         // Hacc-specific
        std::vector<String> keys () const;
        void set_keys (const std::vector<String>&) const;
        size_t length () const;
        void set_length (size_t) const;
         // These might require addressability
        Reference attr (String) const;
        Reference elem (size_t) const;
        Tree to_tree () const;
        void prepare (Tree) const;
        void fill (Tree, bool force = false) const;
        void finish () const;
        void from_tree (Tree) const;
         // These are primarily for use by files.cpp
         // If the callback returns true, the foreach will be terminated.
        bool foreach_address (const Func<bool (Pointer, Path)>&, Path) const;
         // The callback will be given a Reference to a raw pointer.
        bool foreach_pointer (const Func<bool (Reference, Path)>&, Path) const;
    };

     // This is a dynamically typed object with value-semantics.
     // Or, if you prefer, a dynamically type auto_ptr
    struct Dynamic {
        Type type;
        void* addr;

        Pointer address () const { return Pointer(type, addr); }

        Dynamic (Null n = null) : type(Type::CppType<Unknown>()), addr(null) { }
        Dynamic (const Dynamic& o) :
            type(o.type),
            addr(operator new(type.size()))
        {
            type.construct(addr);
            type.copy_assign(addr, o.addr);
        }
        Dynamic (Dynamic&& o) :
            type(o.type),
            addr(o.addr)
        {
            o.addr = null;
        }
        Dynamic (Type type, void* addr) : type(type), addr(addr) { }
        explicit Dynamic (Type type) :
            type(type), addr(operator new (type.size()))
        {
            type.construct(addr);
        }


        template <class C, class... Args>
        static Dynamic New (Args&&... args) {
            void* p = operator new(sizeof(C));
            new (p) C (std::forward<Args>(args)...);
            return Dynamic(Type::CppType<C>(), p);
        }
        template <class C>
        explicit Dynamic (C&& v) : type(Type::CppType<C>()), addr(new C (std::forward<C>(v))) { }

        ~Dynamic () {
            if (addr) {
                type.destruct(addr);
                operator delete(addr);
            }
        }

        Dynamic& operator = (const Dynamic& o) {
            this->~Dynamic();
            return *new (this) Dynamic (o);
        }
        Dynamic& operator = (Dynamic&& o) {
            this->~Dynamic();
            return *new (this) Dynamic (std::forward<Dynamic>(o));
        }
    };

    namespace X {
        struct Unhaccable_Type : Logic_Error {
            Type type;
            Unhaccable_Type (Type);
        };
        struct Unhaccable_Reference : Logic_Error {
            Reference r;
            String goal;
            Unhaccable_Reference (Reference, String);
        };
        struct Form_Mismatch : Logic_Error {
            Type type;
            Tree tree;
            Form_Mismatch (Type, Tree);
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
        struct Wrong_Size : Logic_Error {
            Type type;
            size_t got;
            size_t expected;
            Wrong_Size (Type, size_t, size_t);
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
        struct Address_Not_Found : Logic_Error {
            Pointer pointer;
            Address_Not_Found (Pointer);
        };
        struct Null_Pointer : Logic_Error {
            Pointer pointer;
            Null_Pointer (Pointer);
        };
    }

}

 // This is so we can use Pointers as the key in an unordered_map
namespace std {
    template <>
    struct hash<hacc::Pointer> {
        typedef hacc::Pointer argument_type;
        typedef size_t result_type;
        result_type operator () (argument_type p) const {
            return std::hash<type_index>()(p.type.cpptype())
                 ^ std::hash<void*>()(p.address);
        }
    };
}

#endif
