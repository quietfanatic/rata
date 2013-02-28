#ifndef HAVE_HACC_HACCABLE_STANDARD_H
#define HAVE_HACC_HACCABLE_STANDARD_H

#include "haccable.h"

namespace hacc {
    template <class C>
    struct named_vector : std::vector<C> {
        C* named (std::string name) {
            for (auto& e : *this)
                if (e.name == name)
                    return &e;
            return NULL;
        }
    };
}

HCB_TEMPLATE_BEGIN(<class C>, std::vector<C>)
    using namespace hacc;
    type_name("std::vector<" + get_type_name<C>() + ">");
    to([](const std::vector<C>& v){
        std::vector<Hacc*> a;
        for (const C& e : v) {
            a.push_back(hacc_from(e));
        }
        return new_hacc(std::move(a));
    });
    update_from([](std::vector<C>& v, Hacc* h){
        if (h->form() != ARRAY) throw hacc::Error("A std::vector<" + get_type_name<C>() + "> cannot be represented by a " + form_name(h->form()) + " hacc.");
        auto ah = h->as_array();
        v.resize(ah->n_elems());
        for (uint i = 0; i < ah->n_elems(); i++) {
            update_from_hacc(v[i], ah->elem(i));
        }
    });
    get_elem([](std::vector<C>& v, size_t index){ return Generic(&v[index]); });
HCB_TEMPLATE_END(<class C>, std::vector<C>)

HCB_TEMPLATE_BEGIN(<class C>, hacc::named_vector<C>)
    using namespace hacc;
    type_name("hacc::named_vector<" + get_type_name<C>() + ">");
    delegate(hcb::template supertype<std::vector<C>>());
    get_elem([](named_vector<C>& v, size_t index){ return Generic(&v[index]); });
    get_attr([](named_vector<C>& v, std::string name){ return Generic(v.named(name)); });
HCB_TEMPLATE_END(<class C>, hacc::named_vector<C>)

HCB_TEMPLATE_BEGIN(<class C>, hacc::Map<C>)
    using namespace hacc;
    type_name("hacc::Map<" + get_type_name<C>() + ">");
    to([](const Map<C>& v){
        Map<Hacc*> o;
        for (auto& pair : v) {
            o.push_back(hacc_attr(pair.first, hacc_from(pair.second)));
        }
        return new_hacc(std::move(o));
    });
    update_from([](Map<C>& v, Hacc* h){
        auto oh = h->as_object();
        v.clear();
        v.resize(oh->n_attrs());
        for (uint i = 0; i < oh->n_attrs(); i++) {
            v[i].first = oh->name_at(i);
            update_from_hacc(v[i].second, oh->value_at(i));
        }
    });
HCB_TEMPLATE_END(<class C>, hacc::Map<C>)

HCB_TEMPLATE_BEGIN(<class C>, std::unordered_map<std::string HCB_COMMA C>)
    using namespace hacc;
    type_name("std::unordered_map<std::string, " + get_type_name<C>() + ">");
    to([](const std::unordered_map<std::string, C>& v){
        Map<Hacc*> o;
        for (auto& pair : v) {
            o.push_back(hacc_attr(pair.first, hacc_from((const C&)pair.second)));
        }
        return new_hacc(std::move(o));
    });
    update_from([](std::unordered_map<std::string, C>& v, Hacc* h){
        auto oh = h->as_object();
        v.clear();
        v.reserve(oh->n_attrs());
        for (uint i = 0; i < oh->n_attrs(); i++) {
            update_from_hacc(v[oh->name_at(i)], oh->value_at(i));
        }
    });
    get_attr([](std::unordered_map<std::string, C>& v, std::string name){
        auto iter = v.find(name);
        if (iter != v.end()) {
            return &iter->second;
        }
        else return (C*)NULL;
    });
HCB_TEMPLATE_END(<class C>, std::unordered_map<std::string HCB_COMMA C>)

HCB_TEMPLATE_BEGIN(<class A HCB_COMMA class B>, std::pair<A HCB_COMMA B>)
    using namespace hacc;
    type_name("std::pair<" + get_type_name<A>() + ", " + get_type_name<B>() + ">");
    elem(member(&std::pair<A, B>::first));
    elem(member(&std::pair<A, B>::second));
HCB_TEMPLATE_END(<class A HCB_COMMA class B>, std::pair<A HCB_COMMA B>)

 // Here's a nice thing to put in find_by_id.
template <class C, class Parent>
void chain_find_by_id (const Func<C* (Parent*, std::string)>& f, std::string sep = ":") {
    hacc::Haccability<C>::find_by_id([f, sep](std::string id){
        size_t seppos = id.find(sep);
        if (seppos == std::string::npos) {
            fprintf(stderr, "(Chained ID did not contain %s)\n", sep.c_str());
            return (C*)NULL;
        }
        std::string pid = id.substr(0, seppos);
        Parent* parent = hacc::find_by_id<Parent>(pid);
        if (!parent) {
            fprintf(stderr, "(Could not find %s with id %s)\n",
                hacc::get_type_name<Parent>().c_str(), pid.c_str()
            );
            return (C*)NULL;
        }
        return f(parent, id.substr(seppos + sep.size()));
    });
}

HCB_TEMPLATE_BEGIN(<class C>, C*)
    type_name(hacc::get_type_name<C>() + "*");
    pointer(hacc::Haccability<C*>::template supertype<C*>());
    pointer_policy(hacc::ASK_POINTEE);
HCB_TEMPLATE_END(<class C>, C*)


#endif
