
 // This file contains simple container classes for the anti-STL elitist

#ifdef HEADER

 // Variable width arrays
template <class T>
struct VArray {
    uint n;
    T* p;
    CE VArray () :n(0), p(NULL) { }
    CE VArray (uint n, T* p) :n(n), p(p) { }

    CE T& operator [] (uint i) const { return p[i]; }

    T* begin () { return p; }
    T* end () { return p; }

    //CE operator T* () const { return p; }
    CE operator uint () const { return n; }
    //CE operator bool () const { return n; }

//    void create (uint newn) {
//        n = newn;
//        p = new T [n];
//    }
//    void destroy () {
//        n = 0;
//        delete[] p;
//        p = 0;
//    }
};
template <class C>
void destroy_VArray (VArray<C> a) {
    delete[] a.p;
}

template <class C>
void destroy_VArray_ptrs (VArray<C*> a) {
    for (auto p = a.begin(); p != a.end(); p++)
        delete *p;
}


 // Basic singly-linked list
template <class C>
struct Link {
    C head;
    Link<C>* tail;
    Link (C head, Link<C>* tail) : head(head), tail(tail) { }

    uint length () {
        return this ? 1 + tail->length() : 0;
    }
    VArray<C> to_VArray () {
        VArray<C> r;
        r.n = length();
        r.p = (C*)malloc(r.n * sizeof(C));
        uint i = 0;
        for (auto a = this; a; a = a->tail) {
            r[i++] = a->head;
        }
        return r;
    }
};
template <class C>
void destroy_Links (Link<C>* l) {
    if (l) {
       destroy_Links(l->tail);
       delete l;
    }
}
template <class C>
void destroy_Links_ptrs (Link<C*>* l) {
    if (l) {
        destroy_Links_ptrs(l->tail);
        delete l->head;
        delete l;
    }
}
template <class C>
void encons (Link<C>*& l, C n) {
    l = new Link<C> (n, l);
}
template <class C>
void build_tail (Link<C>**& t, C n) {
    encons(*t, n);
    t = &(*t)->tail;
}


 // For code size we're just using one hash type
KHASH_MAP_INIT_STR(hash, void*);

 // Significantly simplify the interface to khash
 // Our usage patterns will assume no failures, hence the warnings.
template <class C>
struct Hash;
template <class C>
struct Hash<C*> {
    khash_t(hash)* table;
    Hash () : table(kh_init(hash)) { }
    ~Hash () { kh_destroy(hash, table); }
    void insert (CStr name, C* val) {
        int r;
        auto iter = kh_put(hash, table, name, &r);
        if (r == 0)
            WARN("Warning: Hash<?>::insert overwrote %s.\n", name);
        kh_val(table, iter) = val;
    }
    void remove (CStr name) {
        auto iter = kh_get(hash, table, name);
        if (iter == kh_end(table))
            WARN("Warning: Hash<?>::remove did not find %s.\n", name);
        else kh_del(hash, table, iter);
    }
    bool exists (CStr name) {
        return kh_get(hash, table, name) != kh_end(table);
    }
    C* lookup (CStr name) {
        auto iter = kh_get(hash, table, name);
        if (iter == kh_end(table)) {
            WARN("Warning: Hash<?>::lookup did not find %s.\n", name);
            return NULL;
        }
        return (C*)(kh_val(table, iter));
    }
    inline khiter_t next_khiter (khiter_t iter) {
        while (!kh_exist(table, iter)) iter++;
        return iter;
    }
};


#define FOR_IN_HASH(i, h) for (auto i##_iter = kh_begin((h).table), auto i = (C*)kh_val((h).table, i##_iter); i##_iter != kh_end((h).table); i##_iter = (h).next_khiter(i##_iter), i = (C*)kh_val((h).table, i##_iter))

#else

#ifndef DISABLE_TESTS

Tester containers_tester ("containers", [](){
    plan(7);
    Hash<int*> it;
    ok(!it.exists("xyz"), "Hash<>::exists() reports false on noninserted element");
    it.insert("xyz", new int (234));
    pass("Can insert element into hash");
    ok(it.exists("xyz"), "Hash<>::exists() reports true on inserted element");
    int* p = it.lookup("xyz");
    ok(p, "Lookup of inserted element is not NULL");
    is(*p, 234, "Looked-up element is the one inserted");
    it.remove("xyz");
    pass("Can remove element");
    ok(!it.exists("xyz"), "Hash<>::exists() reports false on removed element");
    delete p;
});

#endif

#endif
