#include "../inc/tree.h"
#include "../inc/getsets.h"
#include "../inc/haccable.h"
#include "../inc/files.h"
#include "types_internal.h"

namespace hacc {

    std::unordered_map<std::type_index, Type>& types_by_cpptype () {
        static std::unordered_map<std::type_index, Type> r;
        return r;
    }
    std::unordered_map<String, Type>& types_by_name () {
        static std::unordered_map<String, Type> r;
        return r;
    }

    Type::Type (const std::type_info& cpptype) {
        auto iter = types_by_cpptype().find(cpptype);
        if (iter != types_by_cpptype().end())
            data = iter->second.data;
        else throw X::No_Type_For_CppType(cpptype);
    }
    Type::Type (String name) {
        auto iter = types_by_name().find(name);
        if (iter != types_by_name().end())
            data = iter->second.data;
        else throw X::No_Type_For_Name(name);
    }
    String Type::name () const { return data->name; }
    const std::type_info& Type::cpptype () const { return *data->cpptype; }
    size_t Type::size () const { return data->size; }
    void Type::construct (void* p) const { data->construct(p); }
    void Type::destruct (void* p) const { data->destruct(p); }
    void Type::copy_construct (void* l, void* r) const { data->copy_construct(l, r); }
    TypeData* typedata_by_cpptype (const std::type_info& cpptype) {
        auto iter = types_by_cpptype().find(cpptype);
        if (iter != types_by_cpptype().end())
            return iter->second.data;
        else return null;
    }

    Reference::Reference (Type type, void* p) : c(p), gs(new GS_ID0(type)) { }
    Reference::operator Pointer () const {
        void* p = address();
        if (p) return Pointer(type(), p);
        else throw X::Unaddressable(*this, "convert to Pointer");
    }

    std::vector<String> Reference::keys () {
        if (type().data->keys) {
            std::vector<String> r;
            get([&](void* p){
                type().data->keys->get(p, [&](void* vp){
                    r = *(std::vector<String>*)vp;
                });
            });
            return r;
        }
        else if (!type().data->attr_list.empty()) {
            std::vector<std::string> r;
            r.reserve(type().data->attr_list.size());
            for (auto& a : type().data->attr_list) {
                r.push_back(a.first);
            }
            return r;
        }
        else if (type().data->delegate) {
            std::vector<std::string> r;
            get([&](void* p){
                r = Reference(p, type().data->delegate).keys();
            });
            return r;
        }
        else return std::vector<String>();
    }
    void Reference::set_keys (const std::vector<String>& keys) {
        if (type().data->keys) {
            mod([&](void* p){
                type().data->keys->set(p, [&](void* vp){
                    *(std::vector<String>*)vp = keys;
                });
            });
        }
        else if (!type().data->attr_list.empty()) {
            for (auto& a : type().data->attr_list) {
                for (auto& k : keys) {
                    if (a.first == k)
                        goto next;
                }
                if (!a.second->optional) {
                    throw X::Missing_Attr(type(), a.first);
                }
                next: { }
            }
        }
        else if (type().data->delegate) {
            mod([&](void* p){
                Reference(p, type().data->delegate).set_keys(keys);
            });
        }
        else if (!keys.empty()) {
            throw X::No_Attrs(type());
        }
    }

    Reference Reference::attr (std::string name) {
        if (type().data->attrs_f) {
            Reference r;
            get([&](void* p){ r = type().data->attrs_f(p, name); });
            return r;
        }
        else if (!type().data->attr_list.empty()) {
            if (void* addr = address()) {
                for (auto& a : type().data->attr_list) {
                    if (a.first == name) {
                        return Reference(addr, a.second);
                    }
                }
                throw X::No_Attr(type(), name);
            }
            else throw X::Unaddressable(*this, "get attr from");
        }
        else if (type().data->delegate) {
            if (void* addr = address()) {
                return Reference(addr, type().data->delegate).attr(name);
            }
            else throw X::Unaddressable(*this, "get delegated attr from");
        }
        else throw X::No_Attrs(type(), name);
    }

    size_t Reference::length () {
        if (type().data->length) {
            size_t r;
            get([&](void* p){
                type().data->length->get(p, [&](void* sp){
                    r = *(size_t*)sp;
                });
            });
            return r;
        }
        else if (!type().data->elem_list.empty()) {
            return type().data->elem_list.size();
        }
        else if (type().data->delegate) {
            size_t r;
            get([&](void* p){
                r = Reference(p, type().data->delegate).length();
            });
            return r;
        }
        else return 0;
    }

    void Reference::set_length (size_t size) {
        if (type().data->length) {
            mod([&](void* p){
                type().data->length->set(p, [&](void* sp){
                    *(size_t*)sp = size;
                });
            });
        }
        else if (!type().data->elem_list.empty()) {
            size_t n = type().data->elem_list.size();
            if (size > n) {
                throw X::Too_Long(type(), size, n);
            }
            else for (size_t i = size; i < n; i++) {
                if (!type().data->elem_list[i]->optional) {
                    throw X::Missing_Elem(type(), i);
                }
            }
        }
        else if (type().data->delegate) {
            mod([&](void* p){
                Reference(p, type().data->delegate).set_length(size);
            });
        }
        else if (size != 0) {
            throw X::No_Elems(type());
        }
    }

    Reference Reference::elem (size_t i) {
        if (type().data->elems_f) {
            Reference r;
            get([&](void* p){ r = type().data->elems_f(p, i); });
            return r;
        }
        else if (!type().data->elem_list.empty()) {
            if (void* addr = address()) {
                if (i <= type().data->elem_list.size())
                    return Reference(addr, type().data->elem_list[i]);
                else throw X::Out_Of_Range(type(), i, type().data->elem_list.size());
            }
            else throw X::Unaddressable(*this, "get elem from");
        }
        else if (type().data->delegate) {
            if (void* addr = address()) {
                return Reference(addr, type().data->delegate).elem(i);
            }
            else throw X::Unaddressable(*this, "get elem from");
        }
        else throw X::No_Elems(type(), i);
    }

    Tree* Reference::to_tree () {
        if (type().data->to_tree) {
            Tree* r;
            get([&](void* p){
                r = type().data->to_tree(p);
            });
            return r;
        }
        else {
            const std::vector<String>& ks = keys();
            if (!ks.empty()) {
                Object o;
                for (auto& k : ks) {
                    o.emplace_back(k, attr(k).to_tree());
                }
                return new Tree(std::move(o));
            }
            else {
                size_t n = length();
                if (n > 0) {
                    Array a (n);
                    for (size_t i = 0; i < n; i++) {
                        a[i] = elem(i).to_tree();
                    }
                    return new Tree(std::move(a));
                }
                else return new Tree(Object());
            }
        }
    }
     // TODO: figure out the proper relationship between delegation
     //  and cascading calls (prepare and finish)
    void Reference::prepare (Tree* h) {
        if (type().data->prepare) {
            mod([&](void* p){ type().data->prepare(p, h); });
        }
        else if (type().data->delegate) {
            mod([&](void* p){
                Reference(p, type().data->delegate).prepare(h);
            });
            return;
        }
        switch (h->form) {
            case OBJECT: {
                size_t n = h->o->size();
                std::vector<String> ks;
                ks.reserve(n);
                for (size_t i = 0; i < n; i++) {
                    ks.push_back((*h->o)[i].first);
                }
                set_keys(ks);
                for (size_t i = 0; i < n; i++) {
                    attr(ks[i]).prepare((*h->o)[i].second);
                }
                break;
            }
            case ARRAY: {
                size_t n = h->a->size();
                set_length(n);
                for (size_t i = 0; i < n; i++) {
                    elem(i).prepare((*h->a)[i]);
                }
                break;
            }
            default: break;
        }
    }

    void Reference::fill (Tree* h) {
        if (type().data->fill) {
            mod([&](void* p){ type().data->fill(p, h); });
        }
         // TODO: this delegation has too high priority
        else if (type().data->delegate) {
            mod([&](void* p){
                Reference(p, type().data->delegate).fill(h);
            });
        }
        else switch (h->form) {
            case OBJECT: {
                for (auto& a : *h->o) {
                    attr(a.first).fill(a.second);
                }
                break;
            }
            case ARRAY: {
                size_t n = h->a->size();
                for (size_t i = 0; i < n; i++) {
                    elem(i).fill((*h->a)[i]);
                }
                break;
            }
            default: break;
        }
    }

    void Reference::finish (Tree* h) {
        if (type().data->delegate && !type().data->finish) {
            mod([&](void* p){
                Reference(p, type().data->delegate).finish(h);
            });
        }
        else {
            switch (h->form) {
                case OBJECT: {
                    for (auto& a : *h->o) {
                        attr(a.first).finish(a.second);
                    }
                    break;
                }
                case ARRAY: {
                    size_t n = h->a->size();
                    for (size_t i = 0; i < n; i++) {
                        elem(i).finish((*h->a)[i]);
                    }
                    break;
                }
                default: break;
            }
            if (type().data->finish) {
                mod([&](void* p){ type().data->finish(p, h); });
            }
        }
    }

    void Reference::from_tree (Tree* h) {
        prepare(h);
        fill(h);
        finish(h);
    }

    void Reference::foreach_address (const Func<void (Pointer, Path*)>& cb, Path* path) {
        if (void* addr = address()) {
            cb(Pointer(type(), addr), path);
            const std::vector<String>& ks = keys();
            if (!ks.empty()) {
                for (auto& k : ks) {
                    attr(k).foreach_address(cb, new Path(path, k));
                }
            }
            else {
                size_t n = length();
                for (size_t i = 0; i < n; i++) {
                    elem(i).foreach_address(cb, new Path(path, i));
                }
            }
        }
    }

    void Reference::foreach_pointer (const Func<void (Reference)>& cb) {
        if (type().data->pointee_type.data)
            cb(*this);
        else if (address()) {
            const std::vector<String>& ks = keys();
            if (!ks.empty()) {
                for (auto& k : ks) {
                    attr(k).foreach_pointer(cb);
                }
            }
            else {
                size_t n = length();
                for (size_t i = 0; i < n; i++) {
                    elem(i).foreach_pointer(cb);
                }
            }
        }
    }

    namespace X {
        static String stos (size_t s) {
            std::stringstream ss;
            ss << s;
            return ss.str();
        }

        Type_Mismatch::Type_Mismatch (Type e, Type g) :
            Logic_Error(
                "Type mismatch: expected "
              + e.name() + " but got " + got.name()
            ), expected(e), got(g)
        { }
        Unaddressable::Unaddressable (Reference r, String goal) :
            Logic_Error(
                "Cannot " + goal + " an unaddressable Reference of type "
              + r.type().name()
              + " hosted by object of type "
              + r.host_type().name()
              + " through " + r.gs->description()
            ), r(r), goal(goal)
        { }
        Missing_Attr::Missing_Attr (Type type, String name) :
            Logic_Error(
                "Missing required attribute \"" + name + "\" of type " + type.name()
            ), type(type), name(name)
        { }
        Missing_Elem::Missing_Elem (Type type, size_t i) :
            Logic_Error(
                "Missing required element " + stos(i)
              + " of type " + type.name()
            ), type(type), index(i)
        { }
        Too_Long::Too_Long (Type type, size_t wanted, size_t max) :
            Logic_Error(
                "Provided length " + stos(wanted)
              + " is too long for type " + type.name()
              + " with maximum size " + stos(max)
            ), type(type), wanted(wanted), maximum(max)
        { }
        No_Attr::No_Attr (Type type, String n) :
            Logic_Error(
                "Attribute \"" + n + "\" does not exist in instance of type " + type.name()
            ), type(type), name(n)
        { }
        Out_Of_Range::Out_Of_Range (Type type, size_t i, size_t len) :
            Logic_Error(
                "Index " + stos(i)
              + " is out of range for instance of type " + type.name()
              + " with length " + stos(len)
            ), type(type), index(i), length(len)
        { }
        No_Attrs::No_Attrs (Type type) :
            Logic_Error(
                "Cannot set the keys of instance of type " + type.name()
              + " because it has no attributes"
            ), type(type)
        { }
        No_Attrs::No_Attrs (Type type, String n) :
            Logic_Error(
                "Cannot get attribute \"" + n + "\" from instance of type " + type.name()
              + " because it has no attributes"
            ), type(type), name(n)
        { }
        No_Elems::No_Elems (Type type) :
            Logic_Error(
                "Cannot set the length of instance of type " + type.name()
              + " because it has no elements"
            ), type(type)
        { }
        No_Elems::No_Elems (Type type, size_t i) :
            Logic_Error(
                "Cannot get element " + stos(i)
              + " from instance of type " + type.name()
              + " because it has no elements"
            ), type(type), index(i)
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

HCB_BEGIN(hacc::Unknown)
    name("hacc::Unknown");
HCB_END(hacc::Unknown)

HCB_BEGIN(hacc::Dynamic)
    name("hacc::Dynamic");
    keys(value_funcs<std::vector<String>>(
        [](const hacc::Dynamic& dyn){
            return std::vector<String>(1, dyn.type.name());
        },
        [](hacc::Dynamic& dyn, std::vector<String> keys){
            if (keys.size() != 1) {
                throw X::Logic_Error("A Dynamic must have one key representing its type");
            }
            Type type (keys[0]);
            dyn.destroy();
            dyn.type = type;
            dyn.addr = malloc(type.size());
            type.construct(dyn.addr); 
        }
    ));
    attrs([](Dynamic& dyn, String name)->Reference{
        if (name == dyn.type.name())
            return dyn.address();
        else return Reference(dyn.address()).attr(name);
    });
    elems([](Dynamic& dyn, size_t index)->Reference{
        return Reference(dyn.address()).elem(index);
    });
HCB_END(hacc::Dynamic)

