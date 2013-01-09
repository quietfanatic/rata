#ifndef HAVE_HACC_HACCABLE_STANDARD_H
#define HAVE_HACC_HACCABLE_STANDARD_H

#include "haccable.h"

template <class C> struct Haccable<C*> : hacc::Haccability<C*> {
    void describe (hacc::Haccer& h, C*& it) {
        h.as_pointer(it);
    }
};

template <class C> struct Haccable<hacc::VArray<C>> : hacc::Haccability<hacc::VArray<C>> {
    void describe (hacc::Haccer& h, hacc::VArray<C>& it) {
        using namespace hacc;
         // This is kind of silly honestly.
        if (Haccer::Validator* v = h.validator()) {
            it.assign(v->hacc.get_array().size(), C());
        }
        for (uint i = 0; h.writer() ? i < it.size() : h.elems_left(); i++) {
            elem(it[i]);
        }
    }
};

template <class C> struct Haccable<hacc::Map<C>> : hacc::Haccability<hacc::Map<C>> {
    void describe (hacc::Haccer& h, hacc::Map<C>& it) {
        using namespace hacc;
         // Okay this is really silly.
        if (Haccer::Writer* w = h.writer()) {
            for (auto iter = it.begin(); iter != it.end(); it++) {
                attr(iter->first, iter->second);
            }
        }
        else if (Haccer::Validator* r = h.validator()) {
            auto& o = r->hacc.get_object();
            for (auto iter = o.begin(); iter != o.end(); iter++) {
                attr(iter->first, it[iter->first]);
            }
        }
        else if (Haccer::Reader* r = h.reader()) {
            auto& o = r->hacc.get_object();
            for (auto iter = o.begin(); iter != o.end(); iter++) {
                attr(iter->first, it[iter->first]);
            }
        }
        else if (Haccer::Finisher* r = h.finisher()) {
            auto& o = r->hacc.get_object();
            for (auto iter = o.begin(); iter != o.end(); iter++) {
                attr(iter->first, it[iter->first]);
            }
        }
    }
};

#endif
