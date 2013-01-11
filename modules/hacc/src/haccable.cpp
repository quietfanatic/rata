#include <unordered_map>
#include "../inc/haccable.h"


#define INIT_SAFE(name, ...) static __VA_ARGS__& name () { static __VA_ARGS__ r; return r; }

namespace hacc {

    // Hahaha, deleting 150 lines of code is so satisfying.

    INIT_SAFE(cpptype_map, std::unordered_map<const std::type_info*, HaccTable*>)

    HaccTable* HaccTable::by_cpptype (const std::type_info& t) {
        auto iter = cpptype_map().find(&t);
        return iter != cpptype_map().end()
            ? iter->second
            : null;
    }
    HaccTable* HaccTable::require_cpptype (const std::type_info& t) {
        auto r = by_cpptype(t);
        if (!r) throw Error("Unhaccable type <mangled: " + String(t.name()) + ">");
        return r;
    }

    HaccTable::HaccTable (const std::type_info& t) : cpptype(t) {
        cpptype_map().emplace(&t, this);
    }

    const Hacc* HaccTable::to_hacc (void* p) {
        if (to_hacc) { to_hacc(p); }
         // TODO: Like an object first
         // TODO: Like an array next
         // TODO: Then like a polymorphic thing
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
        if (update_from_hacc) { update_from_hacc(p, h); }
         // TODO: Like an object first
         // TODO: Like an array next
         // TODO: Then like a polymorphic thing
         // Plain delegation last.
        else if (delegate.mtype) {
            HaccTable* t = HaccTable::require_cpptype(*delegate.mtype);
            delegate.set(p, [t, h](void* mp){ t->update_from_hacc(mp, h); });
        }
        else throw Error("Haccability description for <mangled: " + String(cpptype.name()) + "did not provide any way to update from hacc.");
    }

     // Auto-deallocate a pointer if an exception happens.
    namespace { struct Bomb {
        const Func<void ()>* detonate;
        Bomb (const Func<void ()>& d) :detonate(&d) { }
        ~Bomb () { if (detonate) (*detonate)(); }
        void defuse () { detonate = null; }
    }; }

    void* HaccTable::new_from_hacc (const Hacc* h) {
        void* r = allocate();
        Bomb b ([this, r](){ deallocate(r); });
        update_from_hacc(r, h);
        b.defuse();
        return r;
    }

}
