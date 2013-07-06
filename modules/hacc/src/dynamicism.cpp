#include <sstream>
#include "../inc/tree.h"
#include "../inc/strings.h"  // for path_to_string for diagnostics
#include "../inc/files.h"
#include "../inc/haccable.h"  // We're defining haccability on Dynamic
#include "types_internal.h"

namespace hacc {

    Type GetSet0::type () const { return (*this)->t; }
    Type GetSet0::host_type () const { return (*this)->ht; }
    String GetSet0::description () const { return (*this)->description(); }
    void* GetSet0::address (void* c) const { return (*this)->address(c); }
    void* GetSet0::ro_address (void* c) const { return (*this)->ro_address(c); }
    void GetSet0::get (void* c, void* m) const { return (*this)->get(c, m); }
    void GetSet0::set (void* c, void* m) const { return (*this)->set(c, m); }
    GetSet0& GetSet0::optional () { (*this)->optional = true; return *this; }
    GetSet0& GetSet0::required () { (*this)->optional = false; return *this; }
    GetSet0& GetSet0::readonly () { (*this)->readonly = true; return *this; }

    void* Pointer::address_of_type (Type t) const {
        if (t == Type(type)) {
            return address;
        }
        else throw X::Type_Mismatch(type, t);
    }

    Reference::Reference (Type type, void* p) :
        c(p), gs(type.data->gs_id)
    { }
    Reference::operator Pointer () const {
        void* p = address();
        if (p) return Pointer(type(), p);
        else throw X::Unaddressable(*this, "convert to Pointer");
    }

    void Reference::read (const Func<void (void*)>& f) const {
        if (void* addr = ro_address()) {
            f(addr);
        }
        else {
            type().data->stalloc([&](void* p){
                get(p);
                f(p);
            });
        }
    }
    void Reference::write (const Func<void (void*)>& f) const {
        if (void* addr = address()) {
            f(addr);
        }
        else {
            type().data->stalloc([&](void* p){
                f(p);
                set(p);
            });
        }
    }
    void Reference::mod (const Func<void (void*)>& f) const {
        if (void* addr = address()) {
            f(addr);
        }
        else {
            type().data->stalloc([&](void* p){
                get(p);
                f(p);
                set(p);
            });
        }
    }

    std::vector<String> Reference::keys () const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Type(type());
        if (void* addr = ro_address()) {
            if (auto& gs = type().data->keys) {
                if (void* keys_addr = gs.ro_address(addr)) {
                    return *(std::vector<String>*) keys_addr;
                }
                else {
                    std::vector<String> r;
                    gs.get(addr, &r);
                    return r;
                }
            }
            else if (auto& gs = type().data->delegate) {
                return Reference(addr, gs).keys();
            }
            else {
                std::vector<std::string> r;
                r.reserve(type().data->attr_list.size());
                for (auto& a : type().data->attr_list) {
                    if (!a.second->readonly)
                        r.push_back(a.first);
                }
                return r;
            }
        }
        else throw X::Unaddressable(*this, "get keys from");
    }

    void Reference::set_keys (const std::vector<String>& keys) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Type(type());
        if (void* addr = address()) {
            if (auto& gs = type().data->keys) {
                if (void* keys_addr = gs.address(addr)) {
                    *(std::vector<String>*)keys_addr = keys;
                }
                else {
                    gs.set(addr, (void*)&keys);
                }
            }
            else if (auto& gs = type().data->delegate) {
                Reference(addr, gs).set_keys(keys);
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
            else if (!keys.empty()) {
                throw X::No_Attrs(type());
            }
        }
        else throw X::Unaddressable(*this, "set_keys on");
    }

    Reference Reference::attr (std::string name) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Type(type());
        if (void* addr = address()) {
            if (auto& f = type().data->attrs_f) {
                return f(addr, name);
            }
            else if (auto& gs = type().data->delegate) {
                return Reference(addr, gs).attr(name);
            }
            else if (!type().data->attr_list.empty()) {
                for (auto& a : type().data->attr_list) {
                    if (a.first == name) {
                        return Reference(addr, a.second);
                    }
                }
                throw X::No_Attr(type(), name);
            }
            else throw X::No_Attrs(type(), name);
        }
        else throw X::Unaddressable(*this, "get attr from");
    }

    size_t Reference::length () const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Type(type());
        if (void* addr = ro_address()) {
            if (auto& gs = type().data->length) {
                if (void* length_addr = gs.ro_address(addr)) {
                    return *(size_t*) length_addr;
                }
                else {
                    size_t r;
                    gs.get(addr, &r);
                    return r;
                }
            }
            else if (auto& gs = type().data->delegate) {
                return Reference(addr, gs).length();
            }
            else {
                return type().data->elem_list.size();
            }
        }
        else throw X::Unaddressable(*this, "get length from");
    }

    void Reference::set_length (size_t length) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Type(type());
        if (void* addr = address()) {
            if (auto& gs = type().data->length) {
                if (void* length_addr = gs.address(addr)) {
                    *(size_t*)length_addr = length;
                }
                else {
                    gs.set(addr, &length);
                }
            }
            else if (auto& gs = type().data->delegate) {
                Reference(addr, gs).set_length(length);
            }
            else if (!type().data->elem_list.empty()) {
                size_t n = type().data->elem_list.size();
                if (length > n) {
                    throw X::Too_Long(type(), length, n);
                }
                else for (size_t i = length; i < n; i++) {
                    if (!type().data->elem_list[i]->optional) {
                        throw X::Missing_Elem(type(), i);
                    }
                }
            }
            else if (length != 0) {
                throw X::No_Attrs(type());
            }
        }
        else throw X::Unaddressable(*this, "set_keys on");
    }

    Reference Reference::elem (size_t index) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Type(type());
        if (void* addr = address()) {
            if (auto& f = type().data->elems_f) {
                return f(addr, index);
            }
            else if (auto& gs = type().data->delegate) {
                return Reference(addr, gs).elem(index);
            }
            else if (!type().data->elem_list.empty()) {
                if (index <= type().data->elem_list.size())
                    return Reference(addr, type().data->elem_list[index]);
                else
                    throw X::Out_Of_Range(type(), index, type().data->elem_list.size());
            }
            else throw X::No_Elems(type(), index);
        }
        else throw X::Unaddressable(*this, "get attr from");
    }

    Tree* Reference::to_tree () const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Type(type());
         // First check individual special values.
        if (auto& eq = type().data->eq) {
            Tree* r = null;
            read([&](void* addr){
                for (auto& p : type().data->value_list) {
                    if (eq(addr, p.second.addr)) {
                        r = new Tree(p.first);
                        return;
                    }
                }
            });
            if (r) return r;
        }
         // Then custom to_tree
        if (type().data->to_tree) {
            Tree* r;
            read([&](void* p){
                r = type().data->to_tree(p);
            });
            return r;
        }
         // Then delegation
        else if (auto& gs = type().data->delegate) {
            Tree* r;
            read([&](void* addr){
                r = Reference(addr, gs).to_tree();
            });
            return r;
        }
         // Then raw pointers
        else if (type().data->pointee_type) {
            Tree* r;
            read([&](void* addr){
                Pointer pp (type().data->pointee_type, *(void**)addr);
                Path* path = address_to_path(pp);
                if (!path) throw X::Address_Not_Found(pp);
                r = new Tree(path);
            });
            return r;
        }
         // Then as an object or an array
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
    void Reference::prepare (Tree* h) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Type(type());
        if (type().data->prepare) {
            mod([&](void* p){ type().data->prepare(p, h); });
        }
        else if (type().data->delegate) {
            mod([&](void* p){
                Reference(p, type().data->delegate).prepare(h);
            });
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
            case PATH: {
                if (type().data->pointee_type) {
                    String filename = h->p->root();
                    load(File(filename));
                }
                else throw X::Form_Mismatch(type(), PATH);
                break;
            }
            default: break;
        }
    }

    void Reference::fill (Tree* h) const {
         // First check for special values
        if (h->form == STRING) {
            for (auto& pair : type().data->value_list) {
                if (h->s == pair.first) {
                    set(pair.second.addr);
                    return;
                }
            }
        }
         // Then custom fill function
        if (type().data->fill) {
            mod([&](void* p){
                type().data->fill(p, h);
            });
        }
         // then delegation
        else if (type().data->delegate) {
            mod([&](void* p){
                Reference(p, type().data->delegate).fill(h);
            });
        }
         // Then use attrs, elems, and paths
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
            case PATH: {
                Reference pointee = path_to_reference(h->p);
                if (pointee.type() == type().data->pointee_type) {
                    if (void* pointee_addr = pointee.address()) {
                        set(&pointee_addr);
                    }
                    else throw X::Unaddressable(pointee,
                        "generate pointer through path "
                      + path_to_string(h->p)
                      + " from"
                    );
                }
                else throw X::Type_Mismatch(type().data->pointee_type, pointee.type());
                break;
            }
            default: throw X::Form_Mismatch(type(), h->form);
        }
    }

    void Reference::finish (Tree* h) const {
         // Do delegation only if there's no custom finish function
        if (type().data->delegate && !type().data->finish) {
            mod([&](void* p){
                Reference(p, type().data->delegate).finish(h);
            });
        }
        else {
             // Do attrs and elems before main item
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

    void Reference::from_tree (Tree* h) const {
        prepare(h);
        fill(h);
        finish(h);
    }

    bool Reference::foreach_address (const Func<bool (Pointer, Path*)>& cb, Path* path) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Type(type());
        if (void* addr = address()) {
            if (cb(Pointer(type(), addr), path))
                return true;
            const std::vector<String>& ks = keys();
            if (!ks.empty()) {
                for (auto& k : ks) {
                    Path* newpath = path ? new Path(path, k) : null;
                    if (attr(k).foreach_address(cb, newpath))
                        return true;
                }
            }
            else {
                size_t n = length();
                for (size_t i = 0; i < n; i++) {
                    Path* newpath = path ? new Path(path, i) : null;
                    if (elem(i).foreach_address(cb, newpath))
                        return true;
                }
            }
        }
        return false;
    }

    bool Reference::foreach_pointer (const Func<bool (Reference, Path*)>& cb, Path* path) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Type(type());
        if (type().data->pointee_type) {
            if (cb(*this, path))
                return true;
        }
        else if (address()) {
            const std::vector<String>& ks = keys();
            if (!ks.empty()) {
                for (auto& k : ks) {
                    Path* newpath = path ? new Path(path, k) : null;
                    if (attr(k).foreach_pointer(cb, newpath))
                        return true;
                }
            }
            else {
                size_t n = length();
                for (size_t i = 0; i < n; i++) {
                    Path* newpath = path ? new Path(path, i) : null;
                    if (elem(i).foreach_pointer(cb, newpath))
                        return true;
                }
            }
        }
        return false;
    }

    namespace X {
        static String stos (size_t s) {
            std::stringstream ss;
            ss << s;
            return ss.str();
        }

        Unhaccable_Type::Unhaccable_Type (Type t) :
            Logic_Error(
                "Unhaccable type: " + t.name()
            ), type(t)
        { }

        Form_Mismatch::Form_Mismatch (Type t, Form f) :
            Logic_Error(
                "Form mismatch: type " + t.name()
              + " cannot be represented by a " + form_name(f) + " tree"
            ), type(t), form(f)
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
        Address_Not_Found::Address_Not_Found (Pointer p) :
            Logic_Error(
                "Could not find the path of " + p.type.name()
              + " at " + stos((size_t)p.address)
            ), pointer(p)
        { }
    }

} using namespace hacc;

HCB_BEGIN(hacc::Dynamic)
    name("hacc::Dynamic");
    keys(mixed_funcs<std::vector<String>>(
        [](const hacc::Dynamic& dyn){
            return std::vector<String>(1, dyn.type.name());
        },
        [](hacc::Dynamic& dyn, const std::vector<String>& keys){
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

