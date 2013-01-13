#ifndef HAVE_UTIL_ORGANIZATION_H
#define HAVE_UTIL_ORGANIZATION_H


#include "honestly.h"


 // Intrusive linked lists.

template <class C, uint which = 0>
struct Linkable_Link {
     // There is an extra link at each end of the list.
     // This is so that things can be unlinked without knowing
     // which list they were linked in.
     // It also means you cannot see the end of the list by checking
     // if next is null!  Only the next next will be null.
     // Use is_first and is_last on the linked item instead.  Following
     // the C* next of the last item can lead to memory corruption.
    C* next = NULL;
    C* prev = NULL;
};

template <class C, uint which = 0>
struct Links {
    Linkable_Link<C, which> first_pseudo;
    Linkable_Link<C, which> last_pseudo;
    C* first () const { return empty() ? NULL : first_pseudo.next; }
    C* last () const { return empty() ? NULL : last_pseudo.prev; }
    bool empty () const {
        return first_pseudo.next == static_cast<C*>(last_pseudo);
    }
    operator bool () const { return !empty(); }
    void clear () {
        C* nextp;
        for (C* p = first; p && !p->is_last(); p = nextp) {
            nextp = p->next;
            delete p;
        }
    }
};
// for (C* x = list.first(); x && !x.is_last(); x = x->next) ...

HCB_TEMPLATE_BEGIN(<class C>, Links<C>)
    using namespace hacc;
    to([](const Links<C>& v){
        VArray<const Hacc*> a;
        Bomb b ([&a](){ for (auto& p : a) delete p; });
        for (C* p = v.first; p && !p->is_last(); p = p->next) {
            a.push_back(hacc_from(p));
        }
        b.defuse();
        return new_hacc(std::move(a));
    });
    update_from([](Links<C>& v, const Hacc* h){
        auto ah = h->as_array();
        v.clear();
        for (uint i = 0; i < ah->n_elems(); i++) {
            C* n = new_from_hacc<C>(ah->elem(i));
            n->link(v);
        }
    });
HCB_TEMPLATE_END(<class C>, Links<C>)

template <class C, uint which = 0>
struct Linkable : Linkable_Link<C, which> {
    C* next;
    C* prev;
    bool is_linked () { return next && prev; }
    bool is_first () { return !prev || !prev->prev; }
    bool is_last () { return !next || !next->next; }
    void link_after (Linkable_Link<C, which>* l) {
        unlink();
        next = l->next;
        prev = l;
        l->next->prev = this;
        l->next = this;
    }
    void link_before (Linkable_Link<C, which>* l) {
        unlink();
        prev = l->prev;
        next = l;
        l->prev->next = this;
        l->prev = this;
    }
    void link_first (Links<C, which>& l) {
        link_after(l.first_pseudo);
    }
    void link_last (Links<C, which>& l) {
        link_before(l.last_pseudo);
    }
    void link (Links<C, which>& l) { return link_last(l); }
    void unlink () {
        if (next) { next->prev = prev; next = NULL; }
        if (prev) { prev->next = next; prev = NULL; }
    }
    Linkable () { }
    ~Linkable () { unlink(); }
    Linkable (Links<C, which>& l) { link(l); }
};


 // This is for things that are intended to be registered at init-time
 // from multiple compilation units.

template <class C>
struct Register {
    Register (Init_Safe<Hash<C>> table, Str name, C v) {
        table().emplace(name, v);
    }
};

template <class Base, class T>
struct Register_Type {
    Register (Init_Safe<Hash<const std::type_info*>> table, Str name) {
        table().emplace(name, &typeid(T));
    }
};

 // Just run something at init-time.

template <class C>
struct Init {
    Init (void (*code) ()) { code(); }
};

#endif
