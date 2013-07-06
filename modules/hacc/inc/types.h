#ifndef HAVE_HACC_TYPES_H
#define HAVE_HACC_TYPES_H

#include <typeinfo>
#include <typeindex>
#include "common.h"

namespace hacc {

    struct TypeData;
    struct Type {
        TypeData* data;

        bool initialized () const;
        String name () const;
        const std::type_info& cpptype () const;
        size_t size () const;
        void construct (void*) const;
        void destruct (void*) const;
        void copy_assign (void*, void*) const;
        void stalloc (const Func<void (void*)>&) const;

        Type (TypeData* p) : data(p) { }
         // Find type by name; throws if none found
        explicit Type (String);
         // Find type by cpptype; throws if none found
        explicit Type (const std::type_info&);
         // Find type by cpptype; if none found tries to autovivify
        template <class C>
        static Type CppType ();

        bool operator == (const Type& other) const {
             // TypeDatas are guaranteed to not be duplicated.
            return data == other.data;
        }
    };

    void init ();

     // Conditional operations
    template <class C, bool cons = std::is_default_constructible<C>::value>
    struct Constructibility;
    template <class C, bool assgn = std::is_copy_assignable<C>::value>
    struct Assignability;

     // Internal wrapper for declaring and getting types
    Type _get_type (
        const std::type_info&,
        size_t,
        void (* construct )(void*),
        void (* destruct )(void*),
        void (* copy_assign )(void*, void*),
        void (* stalloc )(const Func<void (void*)>&),
        void (* describe )()
    );
    void _init_type (Type, void (*)());

    namespace X {
        struct No_Type_For_CppType : Logic_Error {
            const std::type_info& cpptype;
            No_Type_For_CppType (const std::type_info&);
        };
        struct No_Type_For_Name : Logic_Error {
            String name;
            No_Type_For_Name (String);
        };
        struct Type_Mismatch : Logic_Error {
            Type expected;
            Type got;
            Type_Mismatch (Type expected, Type got);
        };
        struct Not_Constructible : Logic_Error {
            Type type;
            Not_Constructible (Type);
        };
        struct Not_Assignable : Logic_Error {
            Type type;
            Not_Assignable (Type);
        };
    }

    template <class C>
    struct Constructibility<C, true> {
        static void construct (void* p) {
            new (p) C;
        }
        static void stalloc (const Func<void (void*)>& f) {
            C c;
            f(&c);
        }
    };
    template <class C>
    struct Constructibility<C, false> {
        static void construct (void* p) {
            throw X::Not_Constructible(Type::CppType<C>());
        }
        static void stalloc (const Func<void (void*)>& f) {
            throw X::Not_Constructible(Type::CppType<C>());
        }
    };
    template <class C>
    struct Assignability<C, true> {
        static void assign (void* to, void* from) { *(C*)to = *(C*)from; }
    };
    template <class C>
    struct Assignability<C, false> {
        static void assign (void* to, void* from) { throw X::Not_Assignable(Type::CppType<C>()); }
    };

}

 // This is specialized to make a type haccable.  The mechanisms for
 //  defining haccabilities are in haccable.h.
template <class C> struct Hacc_TypeDecl {
    static hacc::Type get_type () {
        static hacc::Type t = hacc::_get_type(
            typeid(C), sizeof(C),
            hacc::Constructibility<C>::construct,
            [](void* p){ ((C*)p)->~C(); },
            hacc::Assignability<C>::assign,
            hacc::Constructibility<C>::stalloc,
            hacc::null
        );
        return t;
    }
};
namespace hacc {
     // Implementation of Type::CPPType
    template <class C>
    Type Type::CppType () {
        return Hacc_TypeDecl<C>::get_type();
    }
}

#endif
