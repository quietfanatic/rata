#ifndef HAVE_HACC_HACCABLE_STANDARD_H
#define HAVE_HACC_HACCABLE_STANDARD_H

#include "haccable.h"



HCB_TEMPLATE_BEGIN(<class C>, hacc::VArray<C>)
    using namespace hacc;
    to([](const VArray<C>& v){
        VArray<const Hacc*> a;
        Bomb b ([&a](){ for (auto& p : a) delete p; });
        for (const C& e : v) {
            a.push_back(hacc_from(e));
        }
        b.defuse();
        return new_hacc(std::move(a));
    });
    update_from([](VArray<C>& v, const Hacc* h){
        auto ah = h->as_array();
        v.clear();
        v.reserve(ah->n_elems());
        for (uint i = 0; i < ah->n_elems(); i++) {
            v.push_back(hacc_to<C>(ah->elem(i)));
        }
    });
HCB_TEMPLATE_END(<class C>, hacc::VArray<C>)


#endif
