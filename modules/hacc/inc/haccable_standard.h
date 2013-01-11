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

HCB_TEMPLATE_BEGIN(<class C>, hacc::Map<C>)
    using namespace hacc;
    to([](const Map<C>& v){
        Map<const Hacc*> o;
        Bomb b ([&o](){ for (auto& p : o) delete p.second; });
        for (auto& pair : v) {
            o.push_back(std::pair<String, const Hacc*>(pair.first, hacc_from(pair.second)));
        }
        b.defuse();
        return new_hacc(std::move(o));
    });
    update_from([](Map<C>& v, const Hacc* h){
        auto oh = h->as_object();
        v.clear();
        v.reserve(oh->n_attrs());
        for (uint i = 0; i < oh->n_attrs(); i++) {
            v.push_back(std::pair<String, const Hacc*>(oh->name_at(i), hacc_from(oh->value_at(i))));
        }
    });
HCB_TEMPLATE_END(<class C>, hacc::Map<C>)

HCB_TEMPLATE_BEGIN(<class C>, std::unordered_map<std::string HCB_COMMA C>)
    using namespace hacc;
    to([](const std::unordered_map<std::string, C>& v){
        Map<const Hacc*> o;
        Bomb b ([&o](){ for (auto& p : o) delete p.second; });
        for (auto& pair : v) {
            o.push_back(std::pair<String, const Hacc*>(pair.first, hacc_from(pair.second)));
        }
        b.defuse();
        return new_hacc(std::move(o));
    });
    update_from([](std::unordered_map<std::string, C>& v, const Hacc* h){
        auto oh = h->as_object();
        v.clear();
        v.reserve(oh->n_attrs());
        for (uint i = 0; i < oh->n_attrs(); i++) {
            v.emplace(std::pair<String, const Hacc*>(oh->name_at(i), hacc_from(oh->value_at(i))));
        }
    });
HCB_TEMPLATE_END(<class C>, std::unordered_map<std::string HCB_COMMA C>)


#endif
