
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

    //CE operator T* () const { return p; }
    CE operator uint () const { return n; }
    //CE operator bool () const { return n; }

//    void allocate (uint newn) {
//        n = newn;
//        p = new T [n];
//    }
//    void free () {
//        n = 0;
//        delete[] p;
//        p = 0;
//    }
//    void reallocate (uint newn) {
//        if (newn == n) return;
//        n = newn;
//        p = new (p) T [n];
//    }
};

 // Basic singly-linked list
template <class C>
struct Link {
    C head;
    Link<C>* tail;
    Link (C head, Link<C>* tail) : head(head), tail(tail) { }
};


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
        if (!kh_exist(table, iter))
            WARN("Warning: Hash<?>::remove did not find %s.\n", name);
        else kh_del(hash, table, iter);
    }
    bool exists (CStr name) {
        return kh_exist(table, kh_get(hash, table, name));
    }
    C* lookup (CStr name) {
        auto iter = kh_get(hash, table, name);
        if (!kh_exist(table, iter)) {
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
    plan(6);
    Hash<int*> it;
    it.insert("xyz", new int (234));
    pass("Can insert element into hash");
    ok(it.exists("xyz"), "hash exists() reports true on inserted element");
    int* p = it.lookup("xyz");
    ok(p, "Lookup of inserted element is not NULL");
    is(*p, 234, "Looked-up element is the one inserted");
    it.remove("xyz");
    pass("Can remove element");
    ok(!it.exists("xyz"), "hash exists() reports false on removed element");
    delete p;
});

#endif

#endif
