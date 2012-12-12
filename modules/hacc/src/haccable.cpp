
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
    void HaccTable::reg_cpptype (const std::type_info& t) {
        auto result = ht_cpptype_hash().emplace(&t, this);
        if (!result.second) {
            throw Error(String("Uh oh, the Haccability for cpptype <mangled: ") + t.name() + "> was instantiated in two different object files.  Currently, the Hacc library can't handle this situation.  If it's a problem, contact me and I might be able to fix this.");
        }
    }
    void HaccTable::reg_hacctype (String s) {
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

    String HaccTable::get_hacctype () {
        if (hacctype.empty() && calc_hacctype)
            hacctype = calc_hacctype();
        return hacctype;
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

}
