#ifndef HAVE_UTIL_ORGANIZATION_H
#define HAVE_UTIL_ORGANIZATION_H

#include <stdexcept>
#include "hacc/inc/haccable.h"
#include "util/inc/honestly.h"

namespace util {

     // Intrusive linked lists.
    template <class C, uint which = 0>
    struct Link {
        Link<C, which>* next;
        Link<C, which>* prev;
        Link () : next(this), prev(this) { }
        Link (Link<C, which>* l) :
            next(l), prev(l->prev)
        {
            l->prev->next = this;
            l->prev = this;
        }
        bool linked () { return next != this; }
        void unlink () {
            next->prev = prev;
            prev->next = next;
            next = this;
            prev = this;
        }
        void link_after (Link<C, which>* l) {
            next->prev = prev;
            prev->next = next;
            next = l->next;
            prev = l;
            l->next->prev = this;
            l->next = this;
        }
        void link_before (Link<C, which>* l) {
            next->prev = prev;
            prev->next = next;
            next = l;
            prev = l->prev;
            l->prev->next = this;
            l->prev = this;
        }
        void link (Link<C, which>* l) { link_before(l); }
        void link (Link<C, which>& l) { link_before(&l); }

        ~Link () { unlink(); }
    };

    template <class C, uint which = 0>
    struct Links : Link<C, which> {
        using Link<C, which>::next;
        using Link<C, which>::prev;
        C* first () const { return empty() ? NULL : static_cast<C*>(next); }
        C* last () const { return empty() ? NULL : static_cast<C*>(prev); }
        bool empty () const { return next == this; }
         // Minimum necessary to use "for (auto& x : links)"
        struct iterator {
            Link<C, which>* p;
            iterator& operator ++ () { p = p->next; return *this; }
            iterator operator ++ (int) { auto r = *this; p = p->next; return r; }
            bool operator != (const iterator& o) { return p != o.p; }
            C& operator * () { return *static_cast<C*>(p); }
            C* operator -> () { return static_cast<C*>(p); }
        };
        iterator begin () { return iterator{next}; }
        iterator end () { return iterator{this}; }
        size_t count () {
            size_t r = 0;
            for (auto& a : *this) r++;
            return r;
        }
    };

    template <class C, Links<C>&(* all)()>
    struct Linked : Link<C> {
        explicit Linked (bool a = true) { if (a) activate(); }
        void activate () { Link<C>::link(all()); }
        void deactivate () { Link<C>::unlink(); }
    };

}

#endif
