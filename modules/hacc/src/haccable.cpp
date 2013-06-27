#include "../inc/haccable.h"
#include "../inc/files.h"

namespace hacc {

    std::unordered_map<std::type_index, HaccTable*> by_type;
    std::unordered_map<std::string, HaccTable*> by_type_name;

    struct HaccTable {
        const Type* type = NULL;
        size_t cppsize = 0;
        void (* construct )(void*) = NULL;
        void (* destruct )(void*) = NULL;
        Func<std::string ()> name;
        GetSet0* keys;
        Func<Reference (void*, String)> attrs_f;
        std::vector<std::pair<String, GetSet0*>> attr_list;
        GetSet0* length;
        Func<Reference (void*, size_t)> elems_f;
        std::vector<GetSet0*> elem_list;
        Func<Hacc* (void*)> to_hacc;
        Func<void (void*, Hacc*)> from_hacc;
        GetSet0* delegate = NULL;
        Func<void (void*, Hacc*)> prepare;
        Func<void (void*, Hacc*)> finish;
    };
    HaccTable* hacctable_by_type (const Type& t) {
        auto iter = by_type.find(t);
        if (iter == by_type.end()) return NULL;
        else return iter->second;
    }
    HaccTable* hacctable_require_type (const Type& t) {
        if (auto r = hacctable_by_type(t))
            return r;
        else throw Error("Unhaccable type: " + type_name(t));
    }
    HaccTable* hacctable_by_type_name (std::string name) {
        auto iter = by_type_name.find(name);
        if (iter == by_type_name.end()) return NULL;
        else return iter->second;
    }
    HaccTable* hacctable_require_type_name (std::string name) {
        if (auto r = hacctable_by_type_name(name))
            return r;
        else throw Error("No registered type with name: " + name);
    }
    HaccTable* new_hacctable (const Type& t, size_t s, void(* c )(void*), void(* d )(void*)) {
        HaccTable& table = util::annotation<HaccTable>(t);
        table.type = t;
        table.size = s;
        table.construct = c;
        table.destruct = d;
        return &table;
    }
    void _name (HaccTable* t, const Func<String ()>& f) { t->name = f; }
    void _keys (HaccTable* t, GetSet0* gs) { t->keys = gs; }
    void _attrs (HaccTable* t, const Func<Reference (void*, String)>& f) { t->attrs_f = f; }
    void _attr (HaccTable* t, String name, GetSet0* gs) { t->attr_list.emplace_back(name, gs); }
    void _length (HaccTable* t, GetSet0* gs) { t->length = gs; }
    void _elems (HaccTable* t, const Func<Reference (void*, size_t)>& f) { t->elems_f = f; }
    void _elem (HaccTable* t, GetSet0* gs) { t->elem_list.push_back(gs); }
    void _to_hacc (HaccTable* t, const Func<Hacc* (void*)>& f) { t->to_hacc = f; }
    void _from_hacc (HaccTable* t, const Func<void (void*, Hacc*)>& f) { t->from_hacc = f; }
    void _delegate (HaccTable* t, GetSet0* gs) { t->delegate = gs; }
    void _prepare (HaccTable* t, const Func<void (void*, Hacc*)>& f) { t->prepare = f; }
    void _finish (HaccTable* t, const Func<void (void*, Hacc*)>& f) { t->finish = f; }

    std::string type_name (const Type& target) {
        if (auto ht = hacctable_by_type(target)) {
            if (ht->name) {
                return ht->name();
            }
        }
        return "{" + target.name() + "}";
    }

    void require_addressable (Reference x, std::string mess) {
        if (!x.addressable()) throw Error(mess);
    }

    std::vector<std::string> keys (Reference x) {
        HaccTable* ht = hacctable_require_type(x.type());
        if (ht->keys) {
             // Hmm, looks like we will have to do a copy after all
            std::vector<std::string> r;
            x.get([&](void* p){
                ht->keys.get(p, [&](void* vp){
                    r = *(std::vector<std::string>*)vp;
                });
            });
            return r;
        }
        else if (!ht->attr_list.empty()) {
            std::vector<std::string> r;
            r.reserve(ht->attr_list.size());
            for (auto& a : ht->attrs) {
                r.push_back(a.first);
            }
            return r;
        }
        else if (ht->delegate) {
            std::vector<std::string> r;
            x.get([&](void* p){
                r = keys(Reference{p, ht->delegate});
            });
            return r;
        }
        else {
            return std::vector<std::string>();
        }
    }

    Reference attr (Reference x, std::string name) {
        HaccTable* ht = hacctable_require_type(x.type());
        if (ht->attrs_f) {
            Reference r;
            x.get([&](void* p){ r = ht->attrs_f(p, name); });
            return r;
        }
        else if (!ht->attr_list.empty) {
            require_addressable(x, "Unaddressable reference given to attr()");
            for (auto& a : ht->attr_list) {
                if (a.first == name) {
                    return Reference{x.address(), a.second};
                }
            }
        }
        else if (ht->delegate) {
            Reference r;
            x.get([&](void* p){
                r = attr(Reference{p, ht->delegate}, name);
            });
            return r;
        }
        else throw Error("Type " + type_name(x.type()) + " has no attrs.");
    }

    size_t length (Reference x) {
        HaccTable* ht = hacctable_require_type(x.type());
        if (ht->length) {
            size_t r;
            x.get([&](void* p){
                ht->length.get(p, [&](void* sp){
                    r = *(size_t*)sp;
                });
            });
            return r;
        }
        else if (!ht->elem_list.empty()) {
            return ht->elem_list.size();
        }
        else if (ht->delegate) {
            size_t r;
            x.get([&](void* p){
                r = length(Reference{p, ht->delegate})
            });
            return r;
        }
        else {
            return 0;
        }
    }

    Reference elem (Reference x, size_t i) {
        HaccTable* ht = hacctable_require_type(p.type()
        if (ht->elems_f) {
            Reference r;
            x.get([&](void* p){ r = ht->elems_f(p, i); });
            return r;
        }
        else if (!ht->elem_list.empty()) {
            require_addressable(x, "Unaddressable reference given to elem()");
            if (i <= ht->elem_list.size())
                return Reference{x.address(), a.second};
            else throw Error("Elem index out of range");
        }
        else if (ht->delegate) {
            Reference r;
            x.get([&](void* p){
                r = elem(Reference{p, ht->delegate}, i)
            });
            return r;
        }
        else throw Error("Target has no positional elements.");
    }

    Hacc* to_hacc (Reference x) {
        HaccTable* ht = hacctable_require_type(x.type());
        if (ht->to_hacc) {
            Hacc* r;
            x.get([&](void* p){
                r = ht->to_hacc(p);
            });
            return r;
        }
        else if (ht->keys) {
            const std::vector<String>& names = keys(x);
            Object o;
            for (auto& name : names) {
                o.emplace_back(name, to_hacc(attr(x, name)));
            }
            return new Hacc(std::move(o));
        }
        else if (ht->attr_list) {
            require_addressable(x, "Unaddressable reference of object-like type given to to_hacc()");
            Object o;
            for (auto& a : ht->attr_list) {
                o.emplace_back(a.first, to_hacc(Reference{x.address(), a.second}));
            }
            return new Hacc(std::move(o));
        }
        else if (ht->length) {
            size_t n = length(x);
            Array a (n);
            for (size_t i = 0; i < n; i++) {
                a[i] = to_hacc(elem(x, to_hacc(elem(x, i))));
            }
            return new Hacc(std::move(a));
        }
        else if (!ht->elem_list.empty()) {
            require_addressable(x, "Unaddressable reference of array-like type given to to_hacc()");
            size_t n = ht->elem_list.size();
            Array a (n)
            for (size_t i = 0; i < n; i++) {
                a[i] = to_hacc(Reference{x.address(), ht->elems[i]);
            }
            return new Hacc(std::move(a));
        }
        else if (ht->delegate) {
            Hacc* r;
            x.get([&](void* p){
                r = to_hacc(Reference{p, ht->delegate});
            });
            return r;
        }
        else {
            return new Hacc(Object());
        }
    }

    void prepare_from_hacc (Reference x, Hacc* h) {
        HaccTable* ht = hacctable_require_type(x.type());
        if (ht->prepare) {
            x.mod([&](void* p){ ht->prepare(p, h); }
        }
        switch (h->form) {
            case ARRAY: {
                size_t n = h->a.size();
                if (ht->length) {
                    x.set([&](void* p){
                        ht->length.set(p, [&](void* sp){
                            *(size_t)*sp = n;
                        });
                    });
                }
                for (size_t i = 0; i < n; i++) {
                    prepare_from_hacc(elem(x, i));
                }
                break;
            }
            case OBJECT: {
                if (ht->keys) {
                    x.set([&](void* p){
                        ht->keys.set(p, [&](void* vp){
                            auto& v = *reinterpret_cast<std::vector<String>*>(vp);
                            v.reserve(h->o.size());
                            for (auto& a : h->o) {
                                v.push_back(a.first);
                            }
                        });
                    });
                    for (auto& a : h->o) {
                        prepare_from_hacc(attr(x, a.first), a.second);
                    }
                }
                else {
                    require_addressable(x, "Unaddressable reference of object-like type given to from_hacc()");
                    for (auto& ha : ht->attr_list) {
                        for (auto& a : h->o) {
                            if (a.first == ha.first) {
                                prepare_from_hacc(Reference{x.address(), ha.second);
                                goto next;
                            }
                        } // no match
                        if (!ha.second->optional)
                            throw Error("Missing required attribute " + ha.first + " of type " + type_name(x.type()));
                        next: { }
                    }
                }
                break;
            }
            default: break;
        }
    }

    void set_from_hacc (Reference x, Hacc* h) {
        HaccTable* ht = hacctable_require_type(x.type());
        if (ht->from_hacc) {
            x.set([&](void* p){ ht->from_hacc(p, h); }
        }
        else switch (h->form) {
            case ARRAY: {
                size_t n = h->a.size();
                if (n == length(x)) {
                    for (size_t i = 0; i < n; i++) {
                        set_from_hacc(elem(x, i), h->a[i]);
                    }
                }
                else throw Error("Wrong number of elements for array-like type " + type_name(x.type()));
                break;
            }
            case OBJECT: {
                if (ht->keys) {
                    for (auto& a : h->o) {
                        set_from_hacc(attr(x, a.first), a.second);
                    }
                }
                else for (auto& ha : ht->attr_list) {
                    for (auto& a : h->o) {
                        if (a.first == ha.first) {
                            set_from_hacc(Reference{x.address(), ha.second);
                            goto next;
                        }
                    } // no match, but we already threw if we missed a required attribute
                    next: { }
                }
                break;
            }
            default: break;
        }
    }

    void finish_from_hacc (Reference x, Hacc* h) {
        switch (h->form) {
            case ARRAY: {
                size_t n = h->a.size();
                for (size_t i = 0; i < n; i++) {
                    finish_from_hacc(elem(x, i), h->a[i]);
                }
                break;
            }
            case OBJECT: {
                if (ht->keys) {
                    for (auto& a : h->o) {
                        set_from_hacc(attr(x, a.first), a.second);
                    }
                }
                else for (auto& ha : ht->attr_list) {
                    for (auto& a : h->o) {
                        if (a.first == ha.first) {
                            set_from_hacc(Reference{x.address(), ha.second);
                            goto next;
                        }
                    }
                    next: { }
                }
                break;
            }
            default: break;
        }
        if (ht->finish) {
            x.mod([&](void* p){ ht->finish(p, h); });
        }
    }

    void from_hacc (Reference x, Hacc* h) {
        prepare_from_hacc(x, h);
        set_from_hacc(x, h);
        finish_from_hacc(x, h);
    }

    Reference path_to_reference (Path* p, Pointer root) {
        if (p == null) return null;
        switch (p->type) {
             // TODO: figure out when and how to fix TOP
            case TOP: {
                if (root) return root;
                else return null;
            }
            case FILE: {
                if (root) return root;
                else return file(p->s);
            }
            case ATTR: {
                Reference left = path_to_reference(p->target, root);
                if (left && left.addressable()) {
                    return attr(left, p->s);
                }
                else return null;
            }
            case ELEM: {
                Reference left = path_to_reference(p->target, root);
                if (left && left.addressable()
                    return elem(left, p->i);
                else return null;
            }
        }
    }

    std::unordered_map<std::pair<std::type_index, void*>, Path*> addresses;
    std::unordered_set<std::string> files_scanned;

     // TODO: use real paths instead of array haccs
    void scan_addresses (Reference x, Path* path) {
        if (x.addressable) {
            HaccTable* ht = hacctable_require_type(x.type());
            addresses.emplace_back(Pointer(x.type(), x.address()), path);
            size_t n = length(x);
            for (size_t i = 0; i < n; i++) {
                ?scan_addresses(elem(x, i), new Path(path, i));
            }
            for (auto& k : keys(x)) {
                scan_addresses(attr(x, k), new Path(path, k));
            }
        }
    }
    Path* address_to_path (Pointer address, Pointer root, File* file) {
        if (root && file) {
            if (!files_scanned.insert(file->filename).second) {
                scan_addresses(root, new Path (file->filename))
            }
        }
        else {
            for (auto f : loaded_files()) {
                if (!files_scanned.insert(f->filename).second) {
                    scan_addresses(f->data, new Path (f->filename));
                }
            }
        }
        auto iter = addresses.find(address);
        if (iter != addresses.end())
            return iter->second;
        else return null;
    }

    void foreach_pointer_internal (Reference x, const Func<void (Reference)>& callback) {
        HaccTable* ht = hacctable_require_type(x.type());
         // TODO: make this work with unaddressable references
        if (x.addressable()) {
            if (ht->pointer) {
                callback(Reference(x.address(), ht->pointer));
            }
            size_t n = length(x);
            for (size_t i = 0; i < n; i++) {
                foreach_pointer_internal(elem(x, i), callback);
            }
            for (auto& k : keys(x)) {
                foreach_pointer_internal(attr(x, k), callback);
            }
        }
    }
    void foreach_pointer (const Func<void (Reference)>& callback, Pointer root) {
        if (root) {
            foreach_pointer_internal(root, callback);
        }
        else {
            for (auto f : loaded_files()) {
                foreach_pointer_internal(f, callback);
            }
        }
    }

}
