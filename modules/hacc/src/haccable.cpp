#include <unordered_map>
#include "../inc/haccable.h"
#include "../inc/haccable_files.h"
#include "../inc/strings.h"

namespace hacc {

     // The global list of all hacctables.
    typedef std::unordered_map<size_t, HaccTable*> Type_Map;
    static Type_Map& cpptype_map () {
        static Type_Map cpptype_map;
        return cpptype_map;
    }
    static std::unordered_map<String, HaccTable*>& type_name_map () {
        static std::unordered_map<String, HaccTable*> type_name_map;
        return type_name_map;
    }

    HaccTable* HaccTable::by_cpptype (const std::type_info& t) {
        auto& r = cpptype_map()[t.hash_code()];
        if (!r) {
            r = new HaccTable(t);
        }
        return r;
    }
    HaccTable* HaccTable::require_cpptype (const std::type_info& t) {
        if (&t == NULL) throw Error("No HaccTable exists for NULL cpptype.\n");
        auto iter = cpptype_map().find(t.hash_code());
        if (iter == cpptype_map().end()) throw Error("No HaccTable exists for type {" + String(t.name()) + "}");
        return iter->second;
    }
    HaccTable* HaccTable::require_type_name (String name) {
        auto iter = type_name_map().find(name);
        if (iter == type_name_map().end()) throw Error("No type_name \"" + name + "\" was registered.\n");
        return iter->second;
    }
    void HaccTable::reg_type_name (String name) {
        type_name_map().emplace(name, this);
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
    struct read_id_info {
        Hacc* read = null;
        Func<void* ()> get;
        read_id_info (Hacc* read, const Func<void* ()>& get) : read(read), get(get) { }
    };
    static std::unordered_map<String, read_id_info> read_ids;
    static uint reading = 0;
    struct read_lock {
        read_lock () { reading++; }
        ~read_lock () {
            if (!--reading) {
                read_ids.clear();
            }
        }
    };

    Hacc* HaccTable::to_hacc (void* p) {
        write_lock wl;

        Hacc* h = to_hacc_inner(p);
         // Save id if this address has been referenced
        auto& hist = write_history[p];
        hist.written = const_cast<Hacc*>(h);
        hist.table = this;
        if (hist.referenced) {
            hist.written->id = hist.id;
        }
        return h;
    }

    Hacc* HaccTable::to_hacc_inner (void* p) {
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
                Hacc* val;
                gs.get(p, [t, &val](void* mp){ val = t->to_hacc(mp); });
                return new_hacc({hacc_attr(v, val)});
            }
        }
         // Then like an object
        else if (attrs.size()) {
            hacc::Object o;
            for (auto& pair : attrs) {
                HaccTable* t = HaccTable::require_cpptype(*pair.second.mtype);
                pair.second.get(p, [&pair, &o, t](void* mp){ o.emplace_back(pair.first, t->to_hacc(mp)); });
            }
            return new_hacc(std::move(o));
        }
         // Like an array next
        else if (elems.size()) {
            hacc::Array a;
            for (auto& gs : elems) {
                HaccTable* t = HaccTable::require_cpptype(*gs.mtype);
                gs.get(p, [&a, t](void* mp){ a.push_back(t->to_hacc(mp)); });
            }
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
            auto& hist = write_history[pp];
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
                            Hacc* val = sub_t->to_hacc(caster.down(pp));
                            return new_hacc({hacc_attr(pair.first, val)});
                        }
                    }
                    throw Error("Unrecognized subtype " + HaccTable::by_cpptype(*realtype)->get_type_name() + " of " + get_type_name());
                }
            }
            else {
                 // Not sure we should prefer an incantation over an id,
                 //  but this is much easier to implement.
                if (Hacc* h = remember_incantation(pp)) {
                    return h;
                }
                 // Make sure all references to this address are on the same page
                if (!hist.referenced) {
                    hist.referenced = true;
                    hist.id = pointee_t->get_id(pp);
                    if (hist.written)
                        hist.written->id = hist.id;
                    else hist.table = pointee_t;
                }
                return new_hacc(hacc::Ref(hist.id));
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
            Hacc* r;
            delegate.get(p, [t, &r](void* mp){ r = t->to_hacc(mp); });
            return r;
        }
        else throw Error("Haccability description for " + get_type_name() + " did not provide any way to turn into a hacc.");
    }

    void HaccTable::update_from_hacc (void* p, Hacc* h, bool save_id) {
        if (!h) throw Error("update_from_hacc called with NULL pointer for hacc.");
        read_lock rl;

        if (save_id) read_ids.emplace(h->id, read_id_info(h, [p](){ return p; }));
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
    void HaccTable::update_with_getset (void* p, Hacc* h, const GetSet0& gs) {
         // For things with IDs, save the ID temporarily
        if (!h->id.empty()) {
            if (gs.copies_on_set) {
                 // If the get function copies too, all hope is lost.
                 //  ...how did you manage to make a pointer to this in the first place?
                if (gs.copies_on_get) {
                    read_ids.emplace(h->id, read_id_info(h, [this, h]()->void*{throw Error("The " + get_type_name() + " with ID '" + h->id + "' could not be referenced due to too much encapsulation.");}));
                    gs.set(p, [&](void* mp){
                        update_from_hacc_inner(mp, h);
                    });
                }
                else {
                    const GetSet0::Get& get = gs.get;
                    read_ids.emplace(h->id, read_id_info(h, [get, p](){
                        void* newmp;
                        get(p, [&newmp](void* mp){ newmp = mp; });
                        return newmp;
                    }));
                    gs.set(p, [&](void* mp){
                        update_from_hacc_inner(mp, h);
                    });
                }
            }
            else {
                gs.set(p, [&](void* mp){
                    read_ids.emplace(h->id, read_id_info(h, [mp](){ return mp; }));
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
        else {
            gs.set(p, [&](void* mp){ update_from_hacc_inner(mp, h); });
        }
    }

    void HaccTable::update_from_hacc_inner (void* p, Hacc* h) {
        if (!initialized)
            throw Error("Unhaccable type " + get_type_name());
        else if (update_from) {
            switch (h->form()) {
                case ATTRREF:
                case ELEMREF:
                case MACROCALL: h = collapse_hacc(h);
                default: { }
            }
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
                    Hacc* val = oh->value_at(0);
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
                    Hacc* val = new_hacc(Array(ah->a.begin()+1, ah->a.end()));
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
                            *(void**)basep = caster.up(t->new_from_hacc(val));
                        });
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
                        if (iter->second.get) {
                            void* addr = iter->second.get();
                            pointer.set(p, [&addr, this](void* mp){
                                *(void**)mp = addr;
                            });
                        }
                        else throw Error("The id " + id + " has no type information.");
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
            case GENERIC: {
                if (pointer) {
                    Generic g = h->as_generic()->g;
                    if (*g.cpptype == *pointee_type) {
                        pointer.set(p, [&g, this](void* mp){
                            *(void**)mp = g.p;
                        });
                        break;
                    }
                    else {
                        throw Error(
                            "Type mismatch: Expected " + get_type_name()
                          + " but got " + HaccTable::by_cpptype(*g.cpptype)->get_type_name()
                        );
                    }
                }
                else throw Error("Type " + get_type_name() + " cannot be represented by a \"Generic\" Hacc.");
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
            case ATTRREF:
            case ELEMREF:
            case DEREF:
            case MACROCALL: {
                update_from_hacc_inner(p, collapse_hacc(h));
                break;
            }
            case ERROR: throw h->as_error()->e;
            default: throw Error("Oops, a corrupted hacc snuck in somewhere.\n");
        }
        if (finish) finish(p);
    }
    struct daBomb {
        HaccTable* t;
        void* p;
        ~daBomb () { if (p) t->deallocate(p); }
        void defuse () { p = null; }
    };

    void* HaccTable::new_from_hacc (Hacc* h) {
        if (!allocate) throw Error("No known way to allocate a " + get_type_name() + "");
        void* r = allocate();
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

    Generic HaccTable::get_attr (void* p, String name) {
        if (get_attr_p) return get_attr_p(p, name);
        else {
            for (auto& a : attrs) {
                if (a.first == name) {
                    void* mp;
                    a.second.get(p, [&mp](void* mp2){ mp = mp2; });
                    return Generic(*a.second.mtype, mp);
                }
            }
            throw Error("Instance of " + get_type_name() + " has no attr " + name);
        }
    }

    Generic HaccTable::get_elem (void* p, size_t index) {
        if (get_elem_p) return get_elem_p(p, index);
        else {
            if (index < elems.size()) {
                void* mp;
                elems[index].get(p, [&mp](void* mp2){ mp = mp2; });
                return Generic(*elems[index].mtype, mp);
            }
            else {
                throw Error("Index out of range for instance of " + get_type_name());
            }
        }
    }

    void just_assign_id (Hacc* h) {
        if (!h->id.empty()) {
            if (h->type.empty()) {
                read_ids.emplace(h->id, read_id_info{h, null});
            }
            else {
                HaccTable* table = HaccTable::require_type_name(h->type);
                void* p = table->new_from_hacc(h);
                read_ids.emplace(h->id, read_id_info(h, [p](){ return p; }));
            }
        }
        else throw Error("ID assignment expected in a place where one wasn't");
    }

    Hacc* collapse_hacc (Hacc* h, Hacc** return_incantation) {
        if (return_incantation) *return_incantation = NULL;
        switch (h->form()) {
            case MACROCALL: {
                auto mch = static_cast<hacc::Hacc::MacroCall*>(h);
                if (mch->mc.name == "file") {
                    mch->mc.arg = collapse_hacc(mch->mc.arg);
                    if (mch->mc.arg->form() == STRING) {
                        Generic g = generic_from_file(mch->mc.arg->get_string());
                        record_incantation(g.p, mch);
                        if (return_incantation) *return_incantation = mch;
                        return new_hacc(g);
                    }
                    else throw Error("The \"file\" macro can only be called on a string.");
                }
                else if (mch->mc.name == "local") {
                    if (mch->mc.arg->form() == ARRAY) {
                        auto ah = mch->mc.arg->as_array();
                        for (auto& subh : ah->a) {
                            if (&subh == &ah->a.back())
                                return subh;
                            else just_assign_id(subh);
                        }
                    }
                }
                else throw Error("Unrecognized macro \"" + mch->mc.name + "\" (Available: \"file\", \"local\")");
            }
            case ATTRREF: {
                auto arh = static_cast<hacc::Hacc::AttrRef*>(h);
                Hacc* incantation;
                arh->ar.subject = collapse_hacc(arh->ar.subject, &incantation);
                if (arh->ar.subject->form() == GENERIC) {
                    auto gh = static_cast<hacc::Hacc::Generic*>(arh->ar.subject);
                    HaccTable* t = HaccTable::require_cpptype(*gh->g.cpptype);
                    Generic attr_g = t->get_attr(gh->g.p, arh->ar.name);
                    if (incantation) {
                        incantation = new_hacc(AttrRef(incantation, arh->ar.name));
                        record_incantation(attr_g.p, incantation);
                        if (return_incantation) *return_incantation = incantation;
                    }
                    return new_hacc(attr_g);
                }
                else throw Error("Attributes can only be requested from a \"Generic\" hacc, such as produced by file()");
            }
            case ELEMREF: {
                auto erh = static_cast<hacc::Hacc::ElemRef*>(h);
                Hacc* incantation;
                erh->er.subject = collapse_hacc(erh->er.subject, &incantation);
                if (erh->er.subject->form() == GENERIC) {
                    auto gh = static_cast<hacc::Hacc::Generic*>(erh->er.subject);
                    HaccTable* t = HaccTable::require_cpptype(*gh->g.cpptype);
                    Generic elem_g = t->get_elem(gh->g.p, erh->er.index);
                    if (incantation) {
                        incantation = new_hacc(ElemRef(incantation, erh->er.index));
                        record_incantation(elem_g.p, incantation);
                        if (return_incantation) *return_incantation = incantation;
                    }
                    return new_hacc(elem_g);
                }
                else throw Error("Elements can only be requested from a \"Generic\" hacc, such as produced by file()");
            }
            case DEREF: {
                Hacc* subject = h->as_deref()->dr.subject;
                if (subject->form() == REF) {
                    auto iter = read_ids.find(static_cast<hacc::Hacc::Ref*>(subject)->r.id);
                    if (iter == read_ids.end())
                        throw Error("ID " + static_cast<hacc::Hacc::Ref*>(subject)->r.id + " not found in this document.");
                    auto& rid = iter->second;
                    if (rid.read) {
                        if (rid.get && !rid.read->type.empty()) {
                            throw Error("Sorry, derefferencing a typed hacc is NYI");
                        }
                        else {
                            rid.read = collapse_hacc(rid.read);
                            return rid.read;
                        }
                    }
                    else throw Error("Internal oops: A read_id was created without a .read");
                }
                else throw Error("Only a REF hacc can be dereferenced with .^");
            }
             // This will only happen if this ref is being dereffed or similar
            case REF: {
                auto iter = read_ids.find(static_cast<hacc::Hacc::Ref*>(h)->r.id);
                if (iter == read_ids.end())
                    throw Error("ID " + static_cast<hacc::Hacc::Ref*>(h)->r.id + " not found in this document.");
                auto& rid = iter->second;
                if (rid.read) {
                    if (rid.get && !rid.read->type.empty()) {
                        HaccTable* t = HaccTable::require_type_name(rid.read->type);
                        return new_hacc(Generic(t->cpptype, rid.get()));
                    }
                    else {
                        Hacc* incantation;
                        rid.read = collapse_hacc(rid.read, &incantation);
                        if (incantation && return_incantation)
                            *return_incantation = incantation;
                        return rid.read;
                    }
                }
                else throw Error("Internal oops: A read_id was created without a .read");
            }
            default: return h;
        }
    }

    std::unordered_map<void*, Hacc*> incantations;
    void record_incantation (void* p, Hacc* h) {
        incantations.emplace(p, h);
         // If there are multiple incantations they'd better be equivalent.
    }
    Hacc* remember_incantation (void* p) {
        auto iter = incantations.find(p);
        if (iter == incantations.end())
            return NULL;
        else return iter->second;
    }
    void clear_incantations () {
        for (auto& i : incantations) {
            if (i.second->form() == MACROCALL) {
                auto mch = static_cast<Hacc::MacroCall*>(i.second);
                if (mch->mc.name == "file" && mch->mc.arg->form() == STRING) {
                    deallocate_file_object(mch->mc.arg->get_string());
                }
            }
        }
        incantations.clear();
    }

}
