
#include <unordered_map>
#include "../inc/haccable.h"

namespace hacc {
    template <class K> using Hash_by = std::unordered_map<K, HaccTable*>;
    static Hash_by<const std::type_info*>& ht_cpptype_hash () {
        static Hash_by<const std::type_info*> r;
        return r;
    }
    static Hash_by<String>& ht_hacctype_hash () {
        static Hash_by<String> r;
        return r;
    }

    HaccTable* HaccTable::gen (const std::type_info& cpptype, size_t cppsize) {
        auto result = ht_cpptype_hash().emplace(&cpptype, null);
         // Return null if this is a duplicate registration.
        if (result.second) {
            result.first->second = new HaccTable (cpptype, cppsize);
            return result.first->second;
        }
        else return null;
    }
    void HaccTable::add_hacctype (String s) {
        if (hacctype.empty()) hacctype = s;
        auto result = ht_hacctype_hash().emplace(s, this);
        if (!result.second) {
            throw Error("The type '" + s + "' was registered twice, once for cpptype <mangled: " + cpptype.name() + ">, and once for cpptype <mangled: " + ht_hacctype_hash().find(s)->second->cpptype.name() + ">");
        }
    }
    HaccTable* HaccTable::by_cpptype (const std::type_info& t) {
        auto iter = ht_cpptype_hash().find(&t);
        if (iter == ht_cpptype_hash().end()) {
            return null;
        }
        else {
            return iter->second;
        }
    }
    HaccTable* HaccTable::require_cpptype (const std::type_info& t) {
        HaccTable* r = by_cpptype(t);
        if (r) return r;
        throw Error("Type <mangled: " + String(t.name()) + "> has no haccable definition.");
    }
    HaccTable* HaccTable::by_hacctype (String s) {
        auto iter = ht_hacctype_hash().find(s);
        if (iter == ht_hacctype_hash().end()) {
            return null;
        }
        else {
            return iter->second;
        }
    }
    HaccTable* HaccTable::require_hacctype (String s) {
        HaccTable* r = by_hacctype(s);
        if (r) return r;
        throw Error("No haccable type was defined with a type name of '" + s + "'");
    }

    String best_type_name (HaccTable* htp) {
        if (htp) {
            String hts = htp->get_hacctype();
            if (!hts.empty())
                return htp->hacctype;
        }
        return String("<mangled: ") + htp->cpptype.name() + ">";
    }
    String best_type_name (const std::type_info& t) {
        return best_type_name(HaccTable::by_cpptype(t));
    }

    String address_to_id (void* addr) {
        char id [17];
        sprintf(id, "%lx", (unsigned long)addr);
        return String(id, strlen(id));
    }
    void* id_to_address (String id) {
        void* r;
        sscanf(id.c_str(), "%lx", (unsigned long*)&r);
        return r;
    }

    void HaccTable::finalize () {
        if (!haccid) haccid = address_to_id;
        if (!find_by_haccid) find_by_haccid = id_to_address;
    }

    String HaccTable::get_hacctype () {
        if (hacctype.empty() && calc_hacctype)
            hacctype = calc_hacctype();
        if (hacctype.empty()) throw Error("No hacctype was provided for " + best_type_name(this) + ".");
        return hacctype;
    }
    String HaccTable::do_haccid (void* p) {
         // This should be given a default.
        return haccid(p);
    }
    void* HaccTable::do_find_by_haccid (String id) {
        return find_by_haccid(id);
    }
    void* HaccTable::do_allocate () {
        if (allocate) return allocate();
        else throw Error("There is no provided way to allocate a " + best_type_name(this) + ".  A default couldn't be provided because it has no nullary constructor.");
    }
    void HaccTable::do_deallocate (void* p) {
        deallocate(p);
    }
    void HaccTable::do_construct (void* p) {
        if (construct) construct(p);
        else throw Error("There is no provided way to construct a " + best_type_name(this) + ".  A default couldn't be provided because it has no nullary constructor.");
    }
    Hacc HaccTable::do_to_hacc (void* p, write_options opts) {
        if (to_hacc) {
            Hacc r = to_hacc(p);
            r.default_type_id(get_hacctype(), haccid(p));
            r.default_options(options);
            r.add_options(opts);
            return r;
        }
        else throw Error("There is no provided way to turn a " + best_type_name(this) + " into a Hacc.");
    }
    void* HaccTable::get_from_hacc () {
        if (from_hacc) return from_hacc;
        else throw Error("There is no provided way to get a " + best_type_name(this) + " from a Hacc.");
    }
    void HaccTable::do_update_from_hacc (void* p, Hacc h) {
        if (update_from_hacc) update_from_hacc(p, h);
        else throw Error("There is no provided way to update a " + best_type_name(this) + " from a Hacc.");
    }
    void* HaccTable::do_new_from_hacc (Hacc h) {
        if (new_from_hacc) return new_from_hacc(h);
        else if (update_from_hacc) {
            if (allocate) {
                void* r = allocate();
                update_from_hacc(r, h);
                return r;
            }
            else throw Error("There is no provided way to make a new " + best_type_name(this) + " from a Hacc.  One couldn't be made from update_from_hacc because it has no nullary constructor.");
        }
        else throw Error("There is no provided way to make a new " + best_type_name(this) + " from a Hacc.");
    }





}
