#include "types_internal.h"

namespace hacc {

    std::unordered_map<std::type_index, TypeData*>& types_by_cpptype () {
        static std::unordered_map<std::type_index, TypeData*> r;
        return r;
    }
    std::unordered_map<String, TypeData*>& types_by_name () {
        static std::unordered_map<String, TypeData*> r;
        return r;
    }

    Type::Type (String name) {
        auto iter = types_by_name().find(name);
        if (iter != types_by_name().end())
            data = iter->second;
        else throw X::No_Type_For_Name(name);
    }
    Type::Type (const std::type_info& cpptype) {
        auto iter = types_by_cpptype().find(cpptype);
        if (iter != types_by_cpptype().end())
            data = iter->second;
        else throw X::No_Type_For_CppType(cpptype);
    }

    bool Type::initialized () const { return data->initialized; }
    String Type::name () const {
        if (!data->name.empty())
            return data->name;
        else
            return "{" + String(data->cpptype->name()) + "}";
    }
    const std::type_info& Type::cpptype () const { return *data->cpptype; }
    size_t Type::size () const { return data->size; }
    void Type::construct (void* p) const { data->construct(p); }
    void Type::destruct (void* p) const { data->destruct(p); }
    void Type::copy_construct (void* l, void* r) const { data->copy_construct(l, r); }

    Type _get_type (
        const std::type_info& cpptype,
        size_t size,
        void (* construct )(void*),
        void (* destruct )(void*),
        void (* copy_construct )(void*, void*)
    ) {
        auto& td = types_by_cpptype()[cpptype];
        if (!td) {
            td = new TypeData(
                cpptype, size,
                construct, destruct,
                copy_construct
            );
        }
        return td;
    }
    void _init_type (Type t, void(* describe )()) {
        if (!t.data->initialized) {
            t.data->initialized = true;
            describe();
        }
    }

    namespace X {
        Type_Mismatch::Type_Mismatch (Type e, Type g) :
            Logic_Error(
                "Type mismatch: expected "
              + e.name() + " but got " + got.name()
            ), expected(e), got(g)
        { }
        No_Type_For_CppType::No_Type_For_CppType (const std::type_info& t) :
            Logic_Error(
                "No haccable type was declared for C++ type {" + String(t.name()) + "}"
            ), cpptype(t)
        { }
        No_Type_For_Name::No_Type_For_Name (String n) :
            Logic_Error(
                "No haccable type was declared with the name " + n
            ), name(n)
        { }
    }
}
