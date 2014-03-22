#include "hacc/src/types_internal.h"

#include "hacc/inc/haccable.h"

namespace hacc {

    std::vector<TypeData*>& types_to_init () {
        static std::vector<TypeData*> r;
        return r;
    }

    std::unordered_map<std::type_index, TypeData*>& types_by_cpptype () {
        static std::unordered_map<std::type_index, TypeData*> r;
        return r;
    }
    std::unordered_map<String, TypeData*>& types_by_name () {
        static std::unordered_map<String, TypeData*> r;
        return r;
    }

    Type::Type (String name) {
        init();
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
        if (!data)
            return "<No type>";
        else if (data->name)
            return data->name();
        else
            return "{" + String(data->cpptype->name()) + "}";
    }
    const std::type_info& Type::cpptype () const { return *data->cpptype; }
    size_t Type::size () const { return data->size; }
    void Type::construct (void* p) const { data->construct(p); }
    void Type::destruct (void* p) const { data->destruct(p); }
    void Type::copy_assign (void* l, void* r) const { data->copy_assign(l, r); }
    bool Type::can_copy_assign () const { return data->can_copy_assign; }
    void Type::stalloc (const Func<void (void*)>& f) const { data->stalloc(f); }

    void init () {
        if (types_to_init().empty()) return;
        for (size_t i = 0; i < types_to_init().size(); i++) {
            TypeData* td = types_to_init()[i];
            if (td->describe) {
                td->initialized = true;
                td->describe();
            }
        }
        for (size_t i = 0; i < types_to_init().size(); i++) {
            TypeData* td = types_to_init()[i];
            if (!td->initialized) {
                if (td->describe) {
                    td->initialized = true;
                    td->describe();
                }
            }
            if (td->name) {
                types_by_name().emplace(td->name(), td);
            }
        }
        types_to_init().clear();
    }

    Type _get_type (
        const std::type_info& cpptype,
        size_t size,
        void (* construct )(void*),
        void (* destruct )(void*),
        void (* copy_assign )(void*, void*),
        bool assignable,
        void (* stalloc )(const Func<void (void*)>&),
        void (* describe )()
    ) {
        auto& td = types_by_cpptype()[cpptype];
        if (!td) {
            td = new TypeData(
                cpptype, size,
                construct, destruct,
                copy_assign,
                assignable,
                stalloc
            );
        }
        if (describe && !td->initialized)
            td->describe = describe;
        return td;
    }
    void _init_type (Type t, void(* describe )()) {
        if (!t.data->initialized) {
            t.data->initialized = true;
            describe();
        }
    }

    namespace X {
        Type_Mismatch::Type_Mismatch (Type e, Type g, String when) :
            Logic_Error(
                (when.empty()
                    ? "Type mismatch: expected "
                    : "Type mismatch " + when + ": expected ")
              + e.name() + " but got " + g.name()
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
        Not_Constructible::Not_Constructible (Type t) :
            Logic_Error(
                "Cannot construct an object of type " + t.name()
              + " because it has no nullary constructor (AKA default constructor)"
            ), type(t)
        { }
        Not_Assignable::Not_Assignable (Type t) :
            Logic_Error(
                "Cannot assign objects of type " + t.name()
              + " because it has no copy assignment operator"
            ), type(t)
        { }
        Not_Destructible::Not_Destructible (Type t) :
            Logic_Error(
                "Cannot destruct an object of type " + t.name()
              + " because its destructor is non-existant or deleted"
            ), type(t)
        { }
    }
} using namespace hacc;

HACCABLE(Type) {
    name("hacc::Type");
    delegate(value_funcs<std::string>(
        [](const Type& t){ return t.name(); },
        [](Type& t, std::string n){ t = Type(n); }
    ));
}
