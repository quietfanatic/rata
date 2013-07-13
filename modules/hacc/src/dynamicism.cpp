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
    GetSet0& GetSet0::narrow () { (*this)->narrow = true; return *this; }
    GetSet0& GetSet0::prepare () { (*this)->prepare = true; return *this; }

    void* address_of_type_internal (Pointer p, Type t) {
        while (p.type == Type::CppType<Dynamic>()) {
            p = ((Dynamic*)p.address)->address();
        }
        if (t == p.type)
            return p.address;
        else
            return null;
    }

    void* Pointer::address_of_type (Type t) const {
        if (void* r = address_of_type_internal(*this, t)) {
            return r;
        }
        else throw X::Type_Mismatch(t, type, "when converting Pointer to " + t.name() + "*");
    }

    Reference::Reference (Type type, void* p) :
        c(p), gs(type.data->gs_id)
    {
        if (!p) throw X::Null_Pointer(Pointer(type, p));
    }
    Reference::operator Pointer () const {
        void* p = address();
        if (p) {
            if (!type().data)
                throw X::Unaddressable(*this, "<Internal error: this Reference had no type>");
            return Pointer(type(), p);
        }
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

     // Apply a getset to a reference
    Reference chain (const Reference& ref, const GetSet0& gs) {
        if (void* addr = ref.address())
            return Reference(addr, gs);
        else
            return Reference(ref.c, GetSet0(new GS_Chain(ref.gs, gs)));
    }

    std::vector<String> Reference::keys () const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Reference(*this, "get keys from");
        if (auto& gs = type().data->keys) {
            std::vector<String> r;
            chain(*this, gs).get(&r);
            return r;
        }
        else if (auto& gs = type().data->delegate) {
            return chain(*this, gs).keys();
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

    void Reference::set_keys (const std::vector<String>& keys) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Reference(*this, "set keys on");
        if (auto& gs = type().data->keys) {
            chain(*this, gs).set((void*)&keys);
        }
        else if (auto& gs = type().data->delegate) {
            chain(*this, gs).set_keys(keys);
        }
        else if (!type().data->attr_list.empty()) {
            for (auto& a : type().data->attr_list) {
                for (auto& k : keys) {
                    if (a.first == k)
                        goto next;
                }
                if (!a.second->optional && !a.second->readonly) {
                    throw X::Missing_Attr(type(), a.first);
                }
                next: { }
            }
        }
        else if (!keys.empty()) {
            throw X::No_Attrs(type());
        }
    }

    Reference Reference::attr (std::string name) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Reference(*this, "get attr " + name + " from");
         // First try specific attrs
        for (auto& a : type().data->attr_list) {
            if (a.first == name) {
                return chain(*this, a.second);
            }
        }
         // Then custom attrs function
        if (auto& f = type().data->attrs_f) {
            if (void* addr = address()) {
                return f(addr, name);
            }
            else throw X::Unaddressable(*this, "get custom attrs from");
        }
         // Then delegation
        else if (auto& gs = type().data->delegate) {
            return chain(*this, gs).attr(name);
        }
        else if (!type().data->attr_list.empty())
            throw X::No_Attr(type(), name);
        else
            throw X::No_Attrs(type(), name);
    }

    size_t Reference::length () const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Reference(*this, "get length from");
        if (auto& gs = type().data->length) {
            size_t r;
            chain(*this, gs).get(&r);
            return r;
        }
        else if (auto& gs = type().data->delegate) {
            return chain(*this, gs).length();
        }
        else {
            return type().data->elem_list.size();
        }
    }

    void Reference::set_length (size_t length) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Reference(*this, "set length on");
        if (auto& gs = type().data->length) {
            chain(*this, gs).set((void*)&length);
        }
        else if (auto& gs = type().data->delegate) {
            chain(*this, gs).set_length(length);
        }
        else if (!type().data->elem_list.empty()) {
            size_t n = type().data->elem_list.size();
            if (length > n) {
                throw X::Wrong_Size(type(), length, n);
            }
            else for (size_t i = length; i < n; i++) {
                if (!type().data->elem_list[i]->optional) {
                    throw X::Missing_Elem(type(), i);
                }
            }
        }
        else if (length != 0) {
            throw X::No_Elems(type());
        }
    }

    Reference Reference::elem (size_t index) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Reference(*this, "get elem from");
         // First try individual elems
        if (index < type().data->elem_list.size()) {
            return chain(*this, type().data->elem_list[index]);
        }
         // Then custom elems function
        else if (auto& f = type().data->elems_f) {
            if (void* addr = address()) {
                return f(addr, index);
            }
            else throw X::Unaddressable(*this, "get custom attrs from");
        }
         // Then delegation
        else if (auto& gs = type().data->delegate) {
            return chain(*this, gs).elem(index);
        }
        else if (!type().data->elem_list.empty())
            throw X::Out_Of_Range(type(), index, type().data->elem_list.size());
        else
            throw X::No_Elems(type(), index);
    }

    Tree Reference::to_tree () const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Reference(*this, "call to_tree on");
         // First check individual special values.
        if (auto& eq = type().data->eq) {
            Tree r;
            read([&](void* addr){
                for (auto& p : type().data->value_list) {
                    if (eq(addr, p.second.addr)) {
                        r = Tree(p.first);
                        return;
                    }
                }
            });
            if (r.operator->()) return r;
        }
         // Then custom to_tree
        if (type().data->to_tree) {
            Tree r;
            read([&](void* p){
                r = type().data->to_tree(p);
            });
            return r;
        }
         // Then delegation
        else if (auto& gs = type().data->delegate) {
            Tree r;
            read([&](void* addr){
                r = Reference(addr, gs).to_tree();
            });
            return r;
        }
         // Then raw pointers
        else if (type().data->pointee_type) {
            Tree r;
            read([&](void* addr){
                Pointer pp (type().data->pointee_type, *(void**)addr);
                if (pp) {
                    Path path = address_to_path(pp);
                    if (!path) throw X::Address_Not_Found(pp);
                    r = Tree(path);
                }
                else r = Tree(null);
            });
            return r;
        }
         // Then as an object or an array
        else {
            if (!type().data->array) {
                const std::vector<String>& ks = keys();
                if (!ks.empty()) {
                    Object o;
                    for (auto& k : ks) {
                        o.emplace_back(k, attr(k).to_tree());
                    }
                    return Tree(std::move(o));
                }
            }
            size_t n = length();
            if (n > 0) {
                Array a (n);
                for (size_t i = 0; i < n; i++) {
                    a[i] = elem(i).to_tree();
                }
                return Tree(std::move(a));
            }
            else return Tree(Object());
        }
    }
    void Reference::prepare (Tree t) const {
        init();
        if (!type().initialized()) throw X::Unhaccable_Reference(*this, "call from_tree on");
        if (gs->narrow) return;
        if (type().data->prepare) {
            mod([&](void* p){ type().data->prepare(p, t); });
        }
        else if (type().data->delegate) {
            mod([&](void* p){
                Reference(p, type().data->delegate).prepare(t);
            });
        }
        else switch (t.form()) {
            case OBJECT: {
                const Object& o = t.as<const Object&>();
                size_t n = o.size();
                std::vector<String> ks;
                ks.reserve(n);
                for (size_t i = 0; i < n; i++) {
                    ks.push_back(o[i].first);
                }
                set_keys(ks);
                for (size_t i = 0; i < n; i++) {
                    attr(ks[i]).prepare(o[i].second);
                }
                break;
            }
            case ARRAY: {
                const Array& a = t.as<const Array&>();
                size_t n = a.size();
                set_length(n);
                for (size_t i = 0; i < n; i++) {
                    elem(i).prepare(a[i]);
                }
                break;
            }
            case PATH: {
                if (!type().data->prepare) {
                    Path p = t.as<Path>();
                    if (type().data->pointee_type) {
                        String filename = p.root();
                        load(File(filename));
                    }
                    else throw X::Form_Mismatch(type(), t);
                }
                break;
            }
            default: break;
        }
        if (gs->prepare) fill(t, true);
    }

    void Reference::fill (Tree t, bool force) const {
        if (!force && gs->prepare) return;
        if (gs->narrow) {
            mod([&](void* p){
                Reference(type(), p).from_tree(t);
            });
            return;
        }
         // First check for special values
        if (t.form() == STRING) {
            String s = t.as<String>();
            for (auto& pair : type().data->value_list) {
                if (s == pair.first) {
                    set(pair.second.addr);
                    return;
                }
            }
        }
         // Then custom fill function
        if (type().data->fill) {
            mod([&](void* p){
                type().data->fill(p, t);
            });
        }
         // then delegation
        else if (type().data->delegate) {
            mod([&](void* p){
                Reference(p, type().data->delegate).fill(t);
            });
        }
         // Then use attrs, elems, and paths
        else switch (t.form()) {
            case OBJECT: {
                for (auto& a : t.as<const Object&>()) {
                    attr(a.first).fill(a.second);
                }
                break;
            }
            case ARRAY: {
                const Array& a = t.as<const Array&>();
                size_t n = a.size();
                for (size_t i = 0; i < n; i++) {
                    elem(i).fill(a[i]);
                }
                break;
            }
            case PATH: {
                Path path = t.as<Path>();
                Reference pointee = path_to_reference(path);
                if (void* pointee_addr = pointee.address()) {
                    Pointer p (pointee.type(), pointee_addr);
                    if (void* addr = address_of_type_internal(p, type().data->pointee_type)) {
                        set(&addr);
                    }
                    else throw X::Type_Mismatch(
                        type().data->pointee_type,
                        pointee.type(),
                        "when reading from path " + path_to_string(path)
                    );
                }
                else throw X::Unaddressable(pointee,
                    "generate pointer through path "
                  + path_to_string(path)
                  + " from"
                );
                break;
            }
            case NULLFORM: {
                if (type().data->pointee_type) {
                    void* p = null;
                    set(&p);
                }
                else throw X::Form_Mismatch(type(), t);
                break;
            }
            default: throw X::Form_Mismatch(type(), t);
        }
    }

    void Reference::finish () const {
        if (gs->narrow) return;
        if (type().data->finish) {
            mod([&](void* p){ type().data->finish(p); });
        }
        else if (type().data->delegate) {
            mod([&](void* p){
                Reference(p, type().data->delegate).finish();
            });
        }
        else {
             // Do attrs and elems before main item
            std::vector<String> ks = keys();
            if (!ks.empty()) {
                for (auto k : ks) {
                    attr(k).finish();
                }
            }
            else {
                size_t n = length();
                for (size_t i = 0; i < n; i++) {
                    elem(i).finish();
                }
            }
        }
    }

    void Reference::from_tree (Tree t) const {
        prepare(t);
        fill(t);
        finish();
    }

    bool Reference::foreach_address (const Func<bool (Pointer, Path)>& cb, Path path) const {
        init();
        if (void* addr = address()) {
            if (cb(Pointer(type(), addr), path))
                return true;
            if (type().initialized()) {
                const std::vector<String>& ks = keys();
                if (!ks.empty()) {
                    for (auto& k : ks) {
                        Path newpath = path ? Path(path, k) : path;
                        if (attr(k).foreach_address(cb, newpath))
                            return true;
                    }
                }
                else {
                    size_t n = length();
                    for (size_t i = 0; i < n; i++) {
                        Path newpath = path ? Path(path, i) : path;
                        if (elem(i).foreach_address(cb, newpath))
                            return true;
                    }
                }
            }
        }
        return false;
    }

    bool Reference::foreach_pointer (const Func<bool (Reference, Path)>& cb, Path path) const {
        init();
        if (!type().initialized()) return false;
        if (type().data->pointee_type) {
            if (cb(*this, path))
                return true;
        }
        else if (address()) {
            const std::vector<String>& ks = keys();
            if (!ks.empty()) {
                for (auto& k : ks) {
                    Path newpath = path ? Path(path, k) : Path(null);
                    if (attr(k).foreach_pointer(cb, newpath))
                        return true;
                }
            }
            else {
                size_t n = length();
                for (size_t i = 0; i < n; i++) {
                    Path newpath = path ? Path(path, i) : Path(null);
                    if (elem(i).foreach_pointer(cb, newpath))
                        return true;
                }
            }
        }
        return false;
    }

    namespace X {
        Unhaccable_Type::Unhaccable_Type (Type t) :
            Logic_Error(
                "Unhaccable type: " + t.name()
            ), type(t)
        { }
        Unhaccable_Reference::Unhaccable_Reference (Reference r, String goal) :
            Logic_Error(
                "Cannot " + goal + " a Reference of unhaccable type "
              + r.type().name()
              + " hosted by object of type "
              + r.host_type().name()
              + " through " + r.gs->description()
            ), r(r), goal(goal)
        { }
        Form_Mismatch::Form_Mismatch (Type t, Tree tree) :
            Logic_Error(
                "Form mismatch: type " + t.name()
              + " cannot be represented by " + form_name(tree.form()) + " tree "
              + tree_to_string(tree)
            ), type(t), tree(tree)
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
                "Missing required element " + std::to_string(i)
              + " of type " + type.name()
            ), type(type), index(i)
        { }
        Wrong_Size::Wrong_Size (Type type, size_t got, size_t expected) :
            Logic_Error( got > expected ? (
                    "Provided length " + std::to_string(got)
                  + " is too long for type " + type.name()
                  + " with maximum size " + std::to_string(expected)
            ) : got < expected ? (
                    "Provided length " + std::to_string(got)
                  + " is too short for type " + type.name()
                  + " with minimum size " + std::to_string(expected)
            ) : (
                "Somebody threw a Wrong_Size error with equal got and expected values "
              + std::to_string(got) + " >_>")
            ), type(type), got(got), expected(expected)
        { }
        No_Attr::No_Attr (Type type, String n) :
            Logic_Error(
                "Attribute \"" + n + "\" does not exist in instance of type " + type.name()
            ), type(type), name(n)
        { }
        Out_Of_Range::Out_Of_Range (Type type, size_t i, size_t len) :
            Logic_Error(
                "Index " + std::to_string(i)
              + " is out of range for instance of type " + type.name()
              + " with length " + std::to_string(len)
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
                "Cannot get element " + std::to_string(i)
              + " from instance of type " + type.name()
              + " because it has no elements"
            ), type(type), index(i)
        { }
        Address_Not_Found::Address_Not_Found (Pointer p) :
            Logic_Error(
                "Could not find the path of " + p.type.name()
              + " at " + std::to_string((size_t)p.address)
            ), pointer(p)
        { }
        Null_Pointer::Null_Pointer (Pointer p) :
            Logic_Error(
                "Cannot convert to Reference null pointer of type "
              + p.type.name()
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
            dyn = Dynamic(type);
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

