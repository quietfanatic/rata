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
    String HaccTable::get_hacctype () {
        infoize();
        return _hacctype.empty() ? "<mangled: " + String(cpptype.name()) + ">" : _hacctype;
    }
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
        ID_Map id_situation;
        Haccer::Validator validator (t, h, id_situation);
        t->describe(validator, p);
        validator.finish();
         // Fill out the ids that weren't declared in this hacc
        for (auto iter = id_situation.begin(); iter != id_situation.end(); iter++) {
            if (!iter->second) {
                 // Split on \0
                String type = String(iter->first.c_str());
                String id = String(iter->first.c_str() + type.length() + 1);
                HaccTable* t = HaccTable::by_hacctype(type);  // Guaranteed not null
                void* addr = t->g_find_by_haccid(id);
                if (!addr) throw Error("No " + type + " was found with id " + id + ".");
                iter->second = addr;
            }
        }
        Haccer::Reader reader (t, h, id_situation, p);
        t->describe(reader, p);
        reader.finish();
        Haccer::Finisher finisher (t, h, id_situation);
        t->describe(finisher, p);
        finisher.finish();
    }

}
