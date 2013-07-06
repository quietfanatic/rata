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

     // Internal wrapper for declaring and getting types
    Type _get_type (
        const std::type_info&,
        size_t,
        void (* construct )(void*),
        void (* destruct )(void*),
        void (* copy_assign )(void*, void*),
        void (* alloca )(const Func<void (void*)>&),
        void (* describe )()
    );
    void _init_type (Type, void (*)());
     // This is specialized to make a type haccable.  The mechanisms for
     //  defining haccabilities are in haccable.h
    template <class C> struct TypeDecl {
        static Type get_type () {
            static Type t = _get_type(
                typeid(C), sizeof(C),
                [](void* p){ new (p) C; },
                [](void* p){ ((C*)p)->~C(); },
                [](void* to, void* from){ new (to) C (*(const C*)from); },
                [](const Func<void (void*)>& f){ C c; f(&c); },
                null
            );
            return t;
        }
    };

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
    }

     // Implementation of Type::CPPType
    template <class C>
    Type Type::CppType () {
        return TypeDecl<C>::get_type();
    }

}

#endif
