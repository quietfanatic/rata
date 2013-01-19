#include <unordered_map>
#include "../inc/haccable.h"


namespace hacc {

     // The global list of all hacctables.
    typedef std::unordered_map<size_t, HaccTable*> Type_Map;
    static Type_Map& cpptype_map () {
        static Type_Map cpptype_map;
        return cpptype_map;
    }

    HaccTable* HaccTable::by_cpptype (const std::type_info& t) {
        auto& r = cpptype_map()[t.hash_code()];
        if (!r) {
            //fprintf(stderr, " # Creating HaccTable for <mangled: %s>, &typeid: %lx\n", t.name(), (unsigned long)&t);
            r = new HaccTable(t);
        }
        return r;
    }
    HaccTable* HaccTable::require_cpptype (const std::type_info& t) {
        auto iter = cpptype_map().find(t.hash_code());
        if (iter == cpptype_map().end()) throw Error("No HaccTable exists for type <mangled: " + String(t.name()) + ">");
        return iter->second;
    }

    HaccTable::HaccTable (const std::type_info& t) : cpptype(t) { }

     // This is used to coordinate IDs when writing
    struct write_id_info {
        std::string id;
        HaccTable* table = null;
        Hacc* written = null;
        bool referenced = false;
    };
    static std::unordered_map<void*, write_id_info> write_history;
    static std::vector<Func<void ()>> delayed_updates;
    static uint writing = 0;
    struct write_lock {
        write_lock () { writing++; }
        ~write_lock () { if (!--writing) write_history.clear(); }
    };
     // These are used to make sure pointers are updated after the pointed-to data
    static std::unordered_map<String, Func<void* ()>> read_ids;
    static uint reading = 0;
    struct read_lock {
        read_lock () { reading++; }
        ~read_lock () {
            if (!--reading) {
                read_ids.clear();
            }
        }
    };

    const Hacc* HaccTable::to_hacc (void* p) {
        write_lock wl;

        const Hacc* h = to_hacc_inner(p);
         // Save id if this address has been referenced
        auto& hist = write_history[p];
        hist.written = const_cast<Hacc*>(h);
        hist.table = this;
        if (hist.referenced) {
            hist.written->id = hist.id;
        }
        return h;
    }

    const Hacc* HaccTable::to_hacc_inner (void* p) {
        if (!initialized)
            throw Error("Unhaccable type " + get_type_name());
        else if (to) { return to(p); }
         // Like a union first
        else if (variants.size() && select_variant) {
            String v = select_variant(p);
            auto iter = variants.find(v);
            if (iter == variants.end()) {
                throw Error("Selected variant '" + v + "' was not listed in variants of " + get_type_name() + ".");
            }
            else {
                auto& gs = iter->second;
                HaccTable* t = HaccTable::require_cpptype(*gs.mtype);
                const Hacc* val;
                gs.get(p, [t, &val](void* mp){ val = t->to_hacc(mp); });
                return new_hacc({hacc_attr(v, val)});
            }
        }
         // Then like an object
        else if (attrs.size()) {
            hacc::Object o;
            Bomb b ([&o](){ for (auto& p : o) { p.second->destroy(); delete p.second; } });
            for (auto& pair : attrs) {
                HaccTable* t = HaccTable::require_cpptype(*pair.second.mtype);
                pair.second.get(p, [&pair, &o, t](void* mp){ o.emplace_back(pair.first, t->to_hacc(mp)); });
            }
            b.defuse();
            return new_hacc(std::move(o));
        }
         // Like an array next
        else if (elems.size()) {
            hacc::Array a;
            Bomb b ([&a](){ for (auto& p : a) { p->destroy(); delete p; } });
            for (auto& gs : elems) {
                HaccTable* t = HaccTable::require_cpptype(*gs.mtype);
                gs.get(p, [&a, t](void* mp){ a.push_back(t->to_hacc(mp)); });
            }
            b.defuse();
            return new_hacc(std::move(a));
        }
         // Following a pointer happens somewhere in here.
        else if (pointer) {
             // Get the effective address
            HaccTable* pointee_t = HaccTable::require_cpptype(*pointee_type);
            void* pp;
            pointer.get(p, [&pp](void* mp){ pp = *(void**)mp; });
             // All pointer types can be null
            if (!pp) return new_hacc(null);
             // Make sure all references to this address are on the same page
            auto& hist = write_history[pp];
            if (!hist.referenced) {
                hist.referenced = true;
                hist.id = pointee_t->get_id(pp);
                if (hist.written)
                    hist.written->id = hist.id;
                else hist.table = pointee_t;
            }
            bool should_follow;
            switch (get_pointer_policy()) {
                case ALWAYS_FOLLOW: should_follow = true; break;
                case FOLLOW: should_follow = !hist.written; break;
                default: should_follow = false; break;
            }
            if (should_follow) {
                if (pointee_t->subtypes.empty()) { // Non-polymorphic
                    return pointee_t->to_hacc(pp);
                }
                else { // Polymorphic, needs casting and tagging
                    const std::type_info* realtype = pointee_realtype(pp);
                    for (auto& pair : pointee_t->subtypes) {
                        auto& caster = pair.second;
                        if (realtype == &caster.subtype) {
                            HaccTable* sub_t = HaccTable::require_cpptype(caster.subtype);
                            const Hacc* val = sub_t->to_hacc(caster.down(pp));
                            return new_hacc({hacc_attr(pair.first, val)});
                        }
                    }
                    throw Error("Unrecognized subtype " + HaccTable::by_cpptype(*realtype)->get_type_name() + " of " + get_type_name());
                }
            }
            else {
                return new_hacc(hacc::Ref(hist.id, pp));
            }
        }
        else if (value_name) {
            return new_hacc(Ref(value_name(p)));
        }
        else if (empty) {
            return new_hacc(Array());
        }
         // Plain delegation last.
        else if (delegate) {
            HaccTable* t = HaccTable::require_cpptype(*delegate.mtype);
            const Hacc* r;
            delegate.get(p, [t, &r](void* mp){ r = t->to_hacc(mp); });
            return r;
        }
        else throw Error("Haccability description for " + get_type_name() + " did not provide any way to turn into a hacc.");
    }

    void HaccTable::update_from_hacc (void* p, const Hacc* h, bool save_id) {
        if (!h) throw Error("update_from_hacc called with NULL pointer for hacc.");
        read_lock rl;

        if (save_id) read_ids.emplace(h->id, [p](){ return p; });
        update_from_hacc_inner(p, h);

        if (reading == 1) {
             // Must be amenable to expansion during iteration.
            for (uint i = 0; i < delayed_updates.size(); i++) {
                delayed_updates[i]();
            }
            delayed_updates.clear();
        }
    }

     // If the set function copies data, thus invalidating the old copy's address,
     //  we must provide a way to get the address of the new data
    void HaccTable::update_with_getset (void* p, const Hacc* h, const GetSet0& gs) {
         // For things with IDs, save the ID temporarily
        if (!h->id.empty()) {
            if (gs.copies_on_set) {
                 // If the get function copies too, all hope is lost.
                 //  ...how did you manage to make a pointer to this in the first place?
                if (gs.copies_on_get) {
                    read_ids.emplace(h->id, null);  // record that this is unavailable
                    gs.set(p, [&](void* mp){
                        update_from_hacc_inner(mp, h);
                    });
                }
                else {
                    const GetSet0::Get& get = gs.get;
                    read_ids.emplace(h->id, [get, p](){
                        void* newmp;
                        get(p, [&newmp](void* mp){ newmp = mp; });
                        return newmp;
                    });
                    gs.set(p, [&](void* mp){
                        update_from_hacc_inner(mp, h);
                    });
                }
            }
            else {
                gs.set(p, [&](void* mp){
                    read_ids.emplace(h->id, [mp](){ return mp; });
                    update_from_hacc_inner(mp, h);
                });
            }
        }
         // For refs, schedule an operation to find the pointee by ID
        else if (h->form() == REF) {
            delayed_updates.emplace_back([this, p, h, &gs](){
                gs.set(p, [this, h](void* mp){
                    update_from_hacc_inner(mp, h);
                });
            });
        }
         // For everything else, just do the thing now.
        else gs.set(p, [&](void* mp){ update_from_hacc_inner(mp, h); });
    }

    void HaccTable::update_from_hacc_inner (void* p, const Hacc* h) {
        if (!initialized)
            throw Error("Unhaccable type " + get_type_name());
        else if (update_from) {
            update_from(p, h);
        }
        else if (delegate) {
            HaccTable* t = HaccTable::require_cpptype(*delegate.mtype);
            t->update_with_getset(p, h, delegate);
        }
        else if (empty) {
             // .......
        }
        else switch (h->form()) {
            case OBJECT: {
                if (attrs.size()) {
                    auto oh = h->as_object();
                    for (auto& pair : attrs) {
                        HaccTable* t = HaccTable::require_cpptype(*pair.second.mtype);
                        if (oh->has_attr(pair.first))
                            t->update_with_getset(p, oh->attr(pair.first), pair.second);
                        else if (pair.second.def.def)
                            pair.second.set(p, [&](void* mp){ pair.second.def.def(mp); });
                        else throw Error("Missing required attribute " + pair.first + " of " + get_type_name());
                    }
                }
                else if (variants.size()) {
                    auto oh = h->as_object();
                    if (oh->n_attrs() != 1) {
                        throw Error("An object Hacc representing a union type (" + get_type_name() + ") must contain only one attribute.");
                    }
                    for (auto& pair : variants) {
                        if (pair.first == oh->name_at(0)) {
                            HaccTable* t = HaccTable::require_cpptype(*pair.second.mtype);
                            t->update_with_getset(p, oh->value_at(0), pair.second);
                            break;
                        }
                    }
                }
                else if (pointer) {
                    HaccTable* pointee_t = HaccTable::require_cpptype(*pointee_type);
                    auto oh = h->as_object();
                    if (oh->n_attrs() != 1) {
                        throw Error("An object Hacc representing a polymorphic type (" + get_type_name() + ") must contain only one attribute.");
                    }
                    String sub = oh->name_at(0);
                    const Hacc* val = oh->value_at(0);
                    auto iter = pointee_t->subtypes.find(sub);
                    if (iter == pointee_t->subtypes.end()) {
                        String r = "Unknown subtype '" + sub + "' of " + pointee_t->get_type_name() + "(Available:";
                        for (auto& p : pointee_t->subtypes) {
                            r += " " + p.first;
                        }
                        throw Error(r);
                    }
                    else {
                        auto& caster = iter->second;
                        HaccTable* t = HaccTable::require_cpptype(caster.subtype);
                        pointer.set(p, [&caster, val, t](void* basep){
                            *(void**)basep = caster.up(t->new_from_hacc(val));
                        });
                        break;
                    }
                }
                else throw Error("Type " + get_type_name() + " cannot be represented by an object Hacc.");
                break;
            }
            case ARRAY: {
                if (elems.size()) {
                    auto ah = h->as_array();
                    for (uint i = 0; i < elems.size(); i++) {
                        HaccTable* t = HaccTable::require_cpptype(*elems[i].mtype);
                        if (i < ah->n_elems())
                            t->update_with_getset(p, ah->elem(i), elems[i]);
                        else if (elems[i].def.def)
                            elems[i].set(p, [&](void* mp){ elems[i].def.def(mp); });
                        else throw Error("Not enough elements to respresent a " + get_type_name());
                    }
                }
                else if (pointer) {
                    HaccTable* pointee_t = HaccTable::require_cpptype(*pointee_type);
                    auto ah = h->as_array();
                    if (ah->n_elems() < 1) {
                        throw Error("An array Hacc representing a polymorphic type (" + get_type_name() + ") must not be empty.");
                    }
                    if (ah->elem(0)->form() != REF) {
                        throw Error("An array Hacc representing a polymorphic type (" + get_type_name() + ") must have a Ref as it's first element.");
                    }
                    String sub = ah->elem(0)->as_ref()->r.id;
                    const Hacc* val = new_hacc(Array(ah->a.begin()+1, ah->a.end()));
                    auto iter = pointee_t->subtypes.find(sub);
                    if (iter == pointee_t->subtypes.end()) {
                        String r = "Unknown subtype '" + sub + "' of " + pointee_t->get_type_name() + " (Available:";
                        for (auto& p : pointee_t->subtypes) {
                            r += " " + p.first;
                        }
                        r += ")";
                        throw Error(r);
                    }
                    else {
                        auto& caster = iter->second;
                        HaccTable* t = HaccTable::require_cpptype(caster.subtype);
                        pointer.set(p, [&caster, val, t](void* basep){
                            void* newp = t->allocate();
                            Bomb b ([newp, t](){ t->deallocate(newp); });
                            t->update_from_hacc_inner(newp, val);
                            *(void**)basep = caster.up(newp);
                            b.defuse();
                        });
                        break;
                    }
                }
                else throw Error("Type " + get_type_name() + " cannot be represented by an array Hacc.");
                break;
            }
            case REF: {
                if (pointer) {
                    HaccTable* pointee_t = HaccTable::require_cpptype(*pointee_type);
                    String id = h->as_ref()->r.id;
                    auto iter = read_ids.find(id);
                    if (iter != read_ids.end()) {
                        if (iter->second) {
                            void* addr = iter->second();
                            pointer.set(p, [&addr, this](void* mp){
                                *(void**)mp = addr;
                            });
                        }
                        else throw Error("The " + pointee_t->get_type_name() + " with ID '" + id + "' could not be referenced due to too much encapsulation.");
                    }
                    else if (void* addr = pointee_t->find_by_id(id)) {
                        pointer.set(p, [&addr, this](void* mp){
                            *(void**)mp = addr;
                        });
                    }
                    else throw Error("No " + pointee_t->get_type_name() + " with ID '" + id + "' could be found.");
                }
                else if (values.size()) {
                    auto iter = values.find(h->as_ref()->r.id);
                    if (iter != values.end())
                        iter->second.def(p);
                    else throw Error("Type " + get_type_name() + " has no value '" + h->as_ref()->r.id);
                }
                else throw Error("Type " + get_type_name() + " cannot be represented by a reference Hacc.");
                break;
            }
            case STRING: throw Error("Type " + get_type_name() + " cannot be represented by a string Hacc.");
            case DOUBLE: throw Error("Type " + get_type_name() + " cannot be represented by a double Hacc.");
            case FLOAT: throw Error("Type " + get_type_name() + " cannot be represented by a float Hacc.");
            case INTEGER: throw Error("Type " + get_type_name() + " cannot be represented by an integer Hacc.");
            case BOOL: throw Error("Type " + get_type_name() + " cannot be represented by a bool Hacc.");
            case NULLFORM: {
                if (pointer) {
                    pointer.set(p, [](void* mp){ *(void**)mp = NULL; });
                }
                else throw Error("Type " + get_type_name() + " cannot be represented by a null Hacc.");
                break;
            }
            case ATTRREF: throw Error("unhaccing ATTRREF is NYI");
            case ELEMREF: throw Error("unhaccing ELEMREF is NYI");
            case MACROCALL: throw Error("unhaccing MACROCALL is NYI");
            case ERROR: throw h->as_error()->e;
            default: throw Error("Oops, a corrupted hacc snuck in somewhere.\n");
        }
    }
    struct daBomb {
        HaccTable* t;
        void* p;
        ~daBomb () { if (p) t->deallocate(p); }
        void defuse () { p = null; }
    };

    void* HaccTable::new_from_hacc (const Hacc* h) {
        if (!allocate) throw Error("No known way to allocate a " + get_type_name() + "");
        void* r = allocate();
         // Bombs are segfaulting again.  *sigh*
        daBomb b {this, r};
        update_from_hacc(r, h);
        b.defuse();
        return r;
    }

    String HaccTable::get_id (void* p) {
        if (get_id_p) return get_id_p(p);
        else {
            char r [17];
            sprintf(r, "@%lx", (unsigned long)p);
            return String((const char*)r);
        }
    }
    void* HaccTable::find_by_id (String s) {
        return find_by_id_p ? find_by_id_p(s) : NULL;
    }
    void* HaccTable::require_id (String s) {
        void* r = find_by_id(s);
        if (!r) throw Error("No " + get_type_name() + " was found with id '" + s + "'.");
        return r;
    }

    uint8 HaccTable::get_pointer_policy () {
        if (pointee_type && pointer_policy == ASK_POINTEE)
            return by_cpptype(*pointee_type)->pointee_policy;
        else return pointer_policy;
    }

    String HaccTable::get_type_name () {
        if (type_name.empty()) return "{" + String(cpptype.name()) + "}";
        else return type_name;
    }

}
