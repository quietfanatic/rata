#include <unordered_map>
#include "../inc/haccable.h"


#define INIT_SAFE(name, ...) static __VA_ARGS__& name () { static __VA_ARGS__ r; return r; }
namespace hacc {

    INIT_SAFE(cpptype_map, std::unordered_map<const std::type_info*, HaccTable*>)
     // This is used to wrangle IDs into pointers when processing haccs.
    std::unordered_map<std::string, void*>* id_map = null;

    HaccTable* HaccTable::by_cpptype (const std::type_info& t) {
        auto& r = cpptype_map()[&t];
        if (!r) {
            //fprintf(stderr, " # Creating HaccTable for <mangled: %s>\n", t.name());
            r = new HaccTable(t);
        }
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
         // Like a union first
        else if (variants.size() && select_variant) {
            String v = select_variant(p);
            auto iter = variants.find(v);
            if (iter == variants.end()) {
                throw Error("Selected variant '" + v + "' was not listed in variants.");
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
            Bomb b ([&o](){ for (auto& p : o) delete p.second; });
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
            Bomb b ([&a](){ for (auto& p : a) delete p; });
            for (auto& gs : elems) {
                HaccTable* t = HaccTable::require_cpptype(*gs.mtype);
                gs.get(p, [&a, t](void* mp){ a.push_back(t->to_hacc(mp)); });
            }
            b.defuse();
            return new_hacc(std::move(a));
        }
         // Following a pointer happens somewhere in here.
        else if (canonical_pointer) {
            HaccTable* pointee_t = HaccTable::require_cpptype(*pointee_type);
            if (pointee_t->subtypes.empty()) { // Non-polymorphic
                const Hacc* r;
                canonical_pointer.get(p, [pointee_t, &r](void* mp){ r = pointee_t->to_hacc(mp); });
                return r;
            }
            else { // Polymorphic
                void* superptr;
                canonical_pointer.get(p, [&superptr](void* mp){ superptr = *(void**)mp; });
                if (!superptr) return new_hacc(null);
                if (!pointee_realtype) throw Error("Uh oh, canonical_pointer was set without pointee_realtype.");
                const std::type_info* realtype = pointee_realtype(superptr);
                for (auto& pair : pointee_t->subtypes) {
                    auto& caster = pair.second;
                    if (realtype == &caster.subtype) {
                        HaccTable* t = HaccTable::require_cpptype(caster.subtype);
                        void* subptr = caster.down(superptr);
                        const Hacc* val = t->to_hacc(subptr);
                        return new_hacc({hacc_attr(pair.first, val)});
                    }
                }
                throw Error("Unrecognized subtype <mangled: " + String(realtype->name()) + "> of <mangled: " + String(cpptype.name()) + ">");
            }
        }
         // Plain delegation last.
        else if (delegate) {
            HaccTable* t = HaccTable::require_cpptype(*delegate.mtype);
            const Hacc* r;
            delegate.get(p, [t, &r](void* mp){ r = t->to_hacc(mp); });
            return r;
        }
        else throw Error("Haccability description for <mangled: " + String(cpptype.name()) + "> did not provide any way to turn into a hacc.");
    }

    void HaccTable::update_from_hacc (void* p, const Hacc* h) {
        if (update_from) { update_from(p, h); }
         // Ah, what the heck, let's do canonical_pointer first.
        else if (canonical_pointer) {
            if (h->form() == OBJECT) {
                auto oh = h->as_object();
                if (oh->n_attrs() != 1) {
                    throw Error("An object Hacc representing a polymorphic type must contain only one attribute.");
                }
                String sub = oh->name_at(0);
                HaccTable* pointee_t = HaccTable::require_cpptype(*pointee_type);
                auto iter = pointee_t->subtypes.find(sub);
                if (iter == pointee_t->subtypes.end()) {
                    throw Error("Unknown subtype '" + sub + "' of <mangled: " + String(pointee_t->cpptype.name()) + ">");
                }
                else {
                    auto& caster = iter->second;
                    HaccTable* t = HaccTable::require_cpptype(caster.subtype);
                    const Hacc* val = oh->value_at(0);
                    canonical_pointer.set(p, [&caster, val, t](void* basep){
                        *(void**)basep = caster.up(t->new_from_hacc(val));
                    });
                    return;
                }
            }
            else throw Error("A polymorphic type can only be represented by an Object hacc or an Array hacc.");
        }
         // Like a union next
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
            else throw Error("A union type can only be represented by an Object hacc or an Array hacc.");
        }
         // Then like an object
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
         // Plain delegation last.
        else if (delegate) {
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
