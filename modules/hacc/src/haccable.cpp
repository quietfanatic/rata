#include <unordered_map>
#include "../inc/haccable.h"


#define INIT_SAFE(name, ...) static __VA_ARGS__& name () { static __VA_ARGS__ r; return r; }

namespace hacc {

    // Hahaha, deleting 150 lines of code is so satisfying.

    INIT_SAFE(cpptype_map, std::unordered_map<const std::type_info*, HaccTable*>)
    INIT_SAFE(hacctype_map, std::unordered_map<String, HaccTable*>)

    HaccTable* HaccTable::by_cpptype (const std::type_info& t) {
        auto iter = cpptype_map().find(&t);
        return iter != cpptype_map().end()
            ? iter->second
            : null;
    }

    HaccTable* HaccTable::by_hacctype (String s) {
        auto iter = hacctype_map().find(s);
        return iter != hacctype_map().end()
            ? iter->second
            : null;
    }

    HaccTable::HaccTable (const std::type_info& t) : cpptype(t) {
        cpptype_map().emplace(&t, this);
    }

    void HaccTable::infoize () { if (infoized) return; info(); infoized = true; }
    String HaccTable::get_hacctype () { infoize(); return _hacctype; }
    void HaccTable::hacctype (String ht) {
        if (_hacctype.empty()) _hacctype = ht;
        hacctype_map().emplace(ht, this);
    }

    void HaccTable::base (HaccTable* b) {
        bases.push_back(b);
    }
    bool HaccTable::has_base (HaccTable* b) {
        infoize();
        if (b == this) return true;
        for (auto i = bases.begin(); i != bases.end(); i++) {
            if ((*i)->has_base(b)) return true;
        }
        return false;
    }
    uint32 HaccTable::get_flags () { infoize(); return _flags; }
    void HaccTable::advertise_id () { _flags |= ADVERTISE_ID; }
    void HaccTable::advertise_type () { _flags |= ADVERTISE_TYPE; }



    void g_update_from_hacc (HaccTable* t, void* p, const Hacc& h) {
        Haccer::Validator validator (h);
        t->describe((void*)&validator, p);
        validator.finish();
        Haccer::Reader reader (h);
        t->describe((void*)&reader, p);
        Haccer::Finisher finisher (h);
        t->describe((void*)&finisher, p);
    }

}
