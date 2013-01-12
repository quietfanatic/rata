#include <unordered_map>
#include "../inc/haccable.h"


#define INIT_SAFE(name, ...) static __VA_ARGS__& name () { static __VA_ARGS__ r; return r; }
namespace hacc {

    INIT_SAFE(cpptype_map, std::unordered_map<const std::type_info*, HaccTable*>)
     // This is used to wrangle IDs into pointers when processing haccs.
    std::unordered_map<std::string, void*>* id_map = null;

    HaccTable* HaccTable::by_cpptype (const std::type_info& t) {
        auto& r = cpptype_map()[&t];
        if (!r) r = new HaccTable(t);
        return r;
    }
    HaccTable* HaccTable::require_cpptype (const std::type_info& t) {
        auto iter = cpptype_map().find(&t);
        if (iter == cpptype_map().end()) throw Error("Unhaccable type <mangled: " + String(t.name()) + ">");
        return iter->second;
    }

    HaccTable::HaccTable (const std::type_info& t) : cpptype(t) { }

    const Hacc* HaccTable::to_hacc (void* p) {
        if (to) { return to(p); }
         // Like an object first
        else if (attrs.size()) {
            hacc::Object o;
            Bomb b ([&o](){ for (auto& p : o) delete p.second; });
            for (auto& pair : attrs) {
                HaccTable* t = HaccTable::require_cpptype(*pair.second.mtype);
                pair.second.get(p, [&pair, &o, t](void* mp){ o.push_back(std::pair<String, const Hacc*>(pair.first, t->to_hacc(mp))); });
            }
            b.defuse();
            return new_hacc(std::move(o));
        }
         // Like an array next
        else if (elems.size()) {
            hacc::Array a;
            Bomb b ([&a](){ for (auto& p : a) delete p; });
            for (auto& gs : elems) {
                HaccTable* t = HaccTable::require_cpptype(*gs.mtype);
                gs.get(p, [&a, t](void* mp){ a.push_back(t->to_hacc(mp)); });
            }
            b.defuse();
            return new_hacc(std::move(a));
        }
         // Then like a union
        else if (variants.size() && select_variant) {
            String v = select_variant(p);
            for (auto& pair : variants) {
                if (pair.first == v) {
                    HaccTable* t = HaccTable::require_cpptype(*pair.second.mtype);
                    const Hacc* val;
                    pair.second.get(p, [t, &val](void* mp){ val = t->to_hacc(mp); });
                    return new_hacc({std::pair<String, const Hacc*>(pair.first, val)});
                }
            }
            throw Error("Selected variant '" + v + "' was not listed in variants.");
        }
         // Plain delegation last.
        else if (delegate.mtype) {
            HaccTable* t = HaccTable::require_cpptype(*delegate.mtype);
            const Hacc* r;
            delegate.get(p, [t, &r](void* mp){ r = t->to_hacc(mp); });
            return r;
        }
        else throw Error("Haccability description for <mangled: " + String(cpptype.name()) + "did not provide any way to turn into a hacc.");
    }

    void HaccTable::update_from_hacc (void* p, const Hacc* h) {
        if (update_from) { update_from(p, h); }
         // Like an object first
        else if (h->form() == OBJECT && attrs.size()) {
            auto oh = h->as_object();
            for (auto& pair : attrs) {
                HaccTable* t = HaccTable::require_cpptype(*pair.second.mtype);
                const Hacc* attr = oh->attr(pair.first);
                pair.second.set(p, [attr, t](void* mp){ t->update_from_hacc(mp, attr); });
            }
        }
         // Like an array next
        else if (h->form() == ARRAY && elems.size()) {
            auto ah = h->as_array();
            for (uint i = 0; i < elems.size(); i++) {
                HaccTable* t = HaccTable::require_cpptype(*elems[i].mtype);
                const Hacc* elem = ah->elem(i);
                elems[i].set(p, [t, elem](void* mp){ t->update_from_hacc(mp, elem); });
            }
        }
         // Then like a union
        else if (variants.size()) {
            if (h->form() == OBJECT) {
                auto oh = h->as_object();
                if (oh->n_attrs() != 1) {
                    throw Error("An object Hacc representing a union type must contain only one attribute.");
                }
                for (auto& pair : variants) {
                    if (pair.first == oh->name_at(0)) {
                        HaccTable* t = HaccTable::require_cpptype(*pair.second.mtype);
                        const Hacc* val = oh->value_at(0);
                        pair.second.set(p, [val, t](void* mp){ t->update_from_hacc(mp, val); });
                        return;
                    }
                }
            }
        }
         // Plain delegation last.
        else if (delegate.mtype) {
            HaccTable* t = HaccTable::require_cpptype(*delegate.mtype);
            delegate.set(p, [t, h](void* mp){ t->update_from_hacc(mp, h); });
        }
        else throw Error("Haccability description for <mangled: " + String(cpptype.name()) + "did not provide any way to update from hacc.");
    }
    void* HaccTable::new_from_hacc (const Hacc* h) {
        void* r = allocate();
        Bomb b ([this, r](){ deallocate(r); });
        update_from_hacc(r, h);
        b.defuse();
        return r;
    }

    String HaccTable::get_id (void* p) {
        return get_id_p ? get_id_p(p) : "";
    }
    void* HaccTable::find_by_id (String s) {
        return find_by_id_p ? find_by_id_p(s) : NULL;
    }
    void* HaccTable::require_id (String s) {
        void* r = find_by_id(s);
        if (!r) throw Error("No <mangled: " + String(cpptype.name()) + "> was found with id '" + s + "'.");
        return r;
    }

}
