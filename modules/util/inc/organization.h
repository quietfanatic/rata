#ifndef HAVE_UTIL_ORGANIZATION_H
#define HAVE_UTIL_ORGANIZATION_H

#include "../../hacc/inc/haccable.h"
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
    Linkable_Link<C, which>* _next;
    Linkable_Link<C, which>* _prev;
    Linkable_Link (Linkable_Link<C, which>* n = NULL, Linkable_Link<C, which>* p = NULL) :
        _next(n), _prev(p)
    { }
};

template <class C, uint which = 0>
struct Links {
    Linkable_Link<C, which> first_pseudo;
    Linkable_Link<C, which> last_pseudo;
    Links () : first_pseudo(&last_pseudo, NULL), last_pseudo(NULL, &first_pseudo) { }
    C* first () const { return empty() ? NULL : static_cast<C*>(first_pseudo._next); }
    C* last () const { return empty() ? NULL : static_cast<C*>(last_pseudo._prev); }
    bool empty () const {
        return first_pseudo._next == &last_pseudo;
    }
    operator bool () const { return !empty(); }
    void destroy_all () {
        for (C* p = first(); p; p = p->next()) {
            delete p;
        }
    }
};
// for (C* x = list.first(); x; x = x->next()) ...

HCB_TEMPLATE_BEGIN(<class C>, Links<C>)
    using namespace hacc;
    to([](const Links<C>& v){
        VArray<const Hacc*> a;
//        Bomb b ([&a](){ for (auto& p : a) delete p; });
        for (C* p = v.first(); p; p = p->next()) {
            a.push_back(hacc_from(p));
        }
//        b.defuse();
        return new_hacc(std::move(a));
    });
    update_from([](Links<C>& v, const Hacc* h){
        auto ah = h->as_array();
        v.destroy_all();
        for (uint i = 0; i < ah->n_elems(); i++) {
            C* n = value_from_hacc<C*>(ah->elem(i));
            if (!n) printf("BLARGH!\n");
            n->link(v);
        }
    });
HCB_TEMPLATE_END(<class C>, Links<C>)

template <class C, uint which = 0>
struct Linkable : Linkable_Link<C, which> {
    using Linkable_Link<C, which>::_next;
    using Linkable_Link<C, which>::_prev;
    bool is_linked () { return _next && _prev; }
    bool is_first () { return !_prev || !_prev->_prev; }
    bool is_last () { return !_next || !_next->_next; }
    C* next () { return is_last() ? NULL : static_cast<C*>(_next); }
    C* prev () { return is_first() ? NULL : static_cast<C*>(_prev); }
    void link_after (Linkable_Link<C, which>* l) {
        unlink();
        _next = l->_next;
        _prev = l;
        l->_next->_prev = this;
        l->_next = this;
    }
    void link_before (Linkable_Link<C, which>* l) {
        unlink();
        _prev = l->_prev;
        _next = l;
        l->_prev->_next = this;
        l->_prev = this;
    }
    void link_first (Links<C, which>& l) {
        link_after(&l.first_pseudo);
    }
    void link_last (Links<C, which>& l) {
        link_before(&l.last_pseudo);
    }
    void link (Links<C, which>& l) { return link_last(l); }
    void unlink () {
        if (_next) { _next->_prev = _prev; _next = NULL; }
        if (_prev) { _prev->_next = _next; _prev = NULL; }
    }
    Linkable () : Linkable_Link<C, which>(NULL, NULL) { }
    ~Linkable () { unlink(); }
    Linkable (Links<C, which>& l) { link(l); }
};


#endif
